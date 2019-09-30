#include "AudioMixer.h"

AudioMixer::AudioMixer()
	: m_initialized(false)
	, m_filterGraph(nullptr)
	, m_audioOutputInfo(nullptr)
{

	m_audioMixInfo.reset(new AudioInfo);
	m_audioMixInfo->name = "amix";

	m_audioSinkInfo.reset(new AudioInfo);
	m_audioSinkInfo->name = "sink";
}

AudioMixer::~AudioMixer()
{

}

int AudioMixer::addAudioInput(uint32_t index, uint32_t samplerate, uint32_t channels, uint32_t bitsPerSample)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_initialized)
	{
		return -1;
	}

	if (m_audioInputInfo.find(index) != m_audioInputInfo.end())
	{
		return -1;
	}

	auto& filterInfo = m_audioInputInfo[index];
	filterInfo.samplerate = samplerate;
	filterInfo.channels = channels;
	filterInfo.bitsPerSample = bitsPerSample;
	filterInfo.name = std::string("input") + std::to_string(index);
	return 0;
}

int AudioMixer::addAudioOutput(uint32_t samplerate, uint32_t channels, uint32_t bitsPerSample)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_initialized)
	{
		return -1;
	}

	m_audioOutputInfo.reset(new AudioInfo);
	m_audioOutputInfo->samplerate = samplerate;
	m_audioOutputInfo->channels = channels;
	m_audioOutputInfo->bitsPerSample = bitsPerSample;
	m_audioOutputInfo->name = "output";

	return 0;
}

int AudioMixer::init()
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_initialized)
	{
		return -1;
	}

	if (m_audioInputInfo.size() == 0)
	{
		return -1;
	}

	m_filterGraph = avfilter_graph_alloc();
	if (m_filterGraph == nullptr)
	{
		return -1;
	}

	char args[512] = {0};

	const AVFilter *amix = avfilter_get_by_name("amix");
	m_audioMixInfo->filterCtx = avfilter_graph_alloc_filter(m_filterGraph, amix, "amix");
	snprintf(args, sizeof(args), "inputs=%d:duration=first:dropout_transition=3", m_audioInputInfo.size());
	if (avfilter_init_str(m_audioMixInfo->filterCtx, args) != 0)
	{
		printf("[AudioMixer] avfilter_init_str(amix) failed.");
		return -1;
	}

	const AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
	m_audioSinkInfo->filterCtx = avfilter_graph_alloc_filter(m_filterGraph, abuffersink, "sink");
	if (avfilter_init_str(m_audioSinkInfo->filterCtx, nullptr) != 0)
	{
		printf("[AudioMixer] avfilter_init_str(abuffersink) failed.");
		return -1;
	}

	for (auto& iter : m_audioInputInfo)
	{
		const AVFilter *abuffer = avfilter_get_by_name("abuffer");
		snprintf(args, sizeof(args),
				"sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
				iter.second.samplerate, 
				getSampleFmtName(iter.second.bitsPerSample).c_str(),
				av_get_default_channel_layout(iter.second.channels));
		printf("[AudioMixer] input(%d) args: %s\n", iter.first, args);

		iter.second.filterCtx = avfilter_graph_alloc_filter(m_filterGraph, abuffer, m_audioOutputInfo->name.c_str());

		if (avfilter_init_str(iter.second.filterCtx, args) != 0)
		{
			printf("[AudioMixer] avfilter_init_str(abuffer) failed.");
			return -1;
		}

		if (avfilter_link(iter.second.filterCtx, 0, m_audioMixInfo->filterCtx, iter.first) != 0)
		{
			printf("[AudioMixer] avfilter_link(abuffer(%d), amix) failed.", iter.first);
			return -1;
		}
	}

	if (m_audioOutputInfo != nullptr)
	{
		const AVFilter *aformat = avfilter_get_by_name("aformat");
		snprintf(args, sizeof(args),
				"sample_fmts=%s:sample_rates=%d:channel_layouts=0x%" PRIx64,
				getSampleFmtName(m_audioOutputInfo->bitsPerSample).c_str(),
				m_audioOutputInfo->samplerate,
				av_get_default_channel_layout(m_audioOutputInfo->channels));
		m_audioOutputInfo->filterCtx = avfilter_graph_alloc_filter(m_filterGraph, aformat, "aformat");

		if (avfilter_init_str(m_audioOutputInfo->filterCtx, args) != 0)
		{
			printf("[AudioMixer] avfilter_init_str(aformat) failed.");
			return -1;
		}

		if (avfilter_link(m_audioMixInfo->filterCtx, 0, m_audioOutputInfo->filterCtx, 0) != 0)
		{
			printf("[AudioMixer] avfilter_link(amix, aformat) failed.");
			return -1;
		}

		if (avfilter_link(m_audioOutputInfo->filterCtx, 0, m_audioSinkInfo->filterCtx, 0) != 0)
		{
			printf("[AudioMixer] avfilter_link(aformat, abuffersink) failed.");
			return -1;
		}
	}

	if (avfilter_graph_config(m_filterGraph, NULL) < 0)
	{
		printf("[AudioMixer] avfilter_graph_config() failed.");
		return -1;
	}

	m_initialized = true;
	return 0;
}

int AudioMixer::exit()
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_initialized)
	{
		for (auto iter : m_audioInputInfo)
		{
			if (iter.second.filterCtx != nullptr)
			{
				avfilter_free(iter.second.filterCtx);
			}
		}

		m_audioInputInfo.clear();

		if (m_audioOutputInfo && m_audioOutputInfo->filterCtx)
		{
			avfilter_free(m_audioOutputInfo->filterCtx);
			m_audioOutputInfo->filterCtx = nullptr;
		}

		if (m_audioMixInfo->filterCtx)
		{
			avfilter_free(m_audioMixInfo->filterCtx);
			m_audioMixInfo->filterCtx = nullptr;
		}

		if (m_audioSinkInfo->filterCtx)
		{
			avfilter_free(m_audioSinkInfo->filterCtx);
			m_audioSinkInfo->filterCtx = nullptr;
		}

		avfilter_graph_free(&m_filterGraph);
		m_filterGraph = nullptr;
		m_initialized = false;
	}

	return 0;
}

int AudioMixer::addFrame(uint32_t index, uint8_t *inBuf, uint32_t size)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (!m_initialized)
	{
		return -1;
	}

	auto iter = m_audioInputInfo.find(index);
	if (iter == m_audioInputInfo.end())
	{
		return -1;
	}

	std::shared_ptr<AVFrame> avFrame(av_frame_alloc(), [](AVFrame *ptr) { av_frame_free(&ptr); });

	avFrame->sample_rate = iter->second.samplerate;
	avFrame->format = getSampleFmt(iter->second.bitsPerSample);
	avFrame->channel_layout = av_get_default_channel_layout(iter->second.channels);
	avFrame->nb_samples = size * 8 / iter->second.bitsPerSample / iter->second.channels;

	av_frame_get_buffer(avFrame.get(), 1);
	memcpy(avFrame->extended_data[0], inBuf, size);

	if (av_buffersrc_add_frame(iter->second.filterCtx, avFrame.get()) != 0)
	{
		return -1;
	}

	return 0;
}

int AudioMixer::getFrame(uint8_t *outBuf, uint32_t maxOutBufSize)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (!m_initialized)
	{
		return -1;
	}

	std::shared_ptr<AVFrame> avFrame(av_frame_alloc(), [](AVFrame *ptr) { av_frame_free(&ptr); });

	int ret = av_buffersink_get_frame(m_audioSinkInfo->filterCtx, avFrame.get());
	if (ret < 0)
	{
		return -1;
	}

	int size = av_samples_get_buffer_size(NULL, avFrame->channels, avFrame->nb_samples, (AVSampleFormat)avFrame->format, 1);

	if (size > (int)maxOutBufSize)
	{
		return 0;
	}

	memcpy(outBuf, avFrame->extended_data[0], size);
	return size;
}

std::string AudioMixer::getSampleFmtName(uint32_t bitsPerSample)
{
	std::string name;
	switch(bitsPerSample)
	{
		case 8:
			name = av_get_sample_fmt_name(AV_SAMPLE_FMT_U8);
			break;
		case 16:
			name = av_get_sample_fmt_name(AV_SAMPLE_FMT_S16);
			break;
		case 32:
			name = av_get_sample_fmt_name(AV_SAMPLE_FMT_S32);
			break;
		default:
			break;
	}

	return name;
}

AVSampleFormat AudioMixer::getSampleFmt(uint32_t bitsPerSample)
{
	switch (bitsPerSample)
	{
	case 8:
		return AV_SAMPLE_FMT_U8;
		break;
	case 16:
		return AV_SAMPLE_FMT_S16;
		break;
	case 32:
		return AV_SAMPLE_FMT_S32;
		break;
	default:
		break;
	}

	return AV_SAMPLE_FMT_NONE;
}