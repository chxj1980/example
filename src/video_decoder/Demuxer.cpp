#include "Demuxer.h"
#include "log.h"

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

Demuxer::Demuxer()
	: m_fmtCtx(NULL)
	, m_videoIndex(-1)
	, m_audioIndex(-1)
{

}

Demuxer::~Demuxer()
{

}

int Demuxer::open(std::string url)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_fmtCtx != NULL)
	{
		LOG("demuxer was opened.");
		return -1;
	}

	int ret = avformat_open_input(&m_fmtCtx, url.c_str(), 0, NULL);
	if (ret != 0)
	{
		AV_LOG(ret, "open %s failed.", url.c_str());
		return -1;
	}

	avformat_find_stream_info(m_fmtCtx, 0);
	av_dump_format(m_fmtCtx, 0, url.c_str(), 0);

	m_videoIndex = av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (m_videoIndex >= 0)
	{
		m_videoStream = m_fmtCtx->streams[m_videoIndex];
	}

	m_audioIndex = av_find_best_stream(m_fmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (m_audioIndex >= 0)
	{
		m_audioStream = m_fmtCtx->streams[m_audioIndex];
	}

	m_url = url;
	return 0;
}

int Demuxer::close()
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_fmtCtx != NULL)
	{
		avformat_close_input(&m_fmtCtx);
		m_fmtCtx = NULL;
	}

	m_videoStream = NULL;
	m_audioStream = NULL;
	m_videoIndex = -1;
	m_audioIndex = -1;
	return 0;
}

int Demuxer::read(AVPacketPtr& pkt)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (!m_fmtCtx) 
	{
		return -1;
	}
	
	pkt.reset(av_packet_alloc(), [](AVPacket *ptr) { av_packet_free(&ptr); });
	int ret = av_read_frame(m_fmtCtx, pkt.get());
	if (ret != 0)
	{
		return -1;
	}

	pkt->pts = (int64_t)(pkt->pts*(1000 * (r2d(m_fmtCtx->streams[pkt->stream_index]->time_base))));
	pkt->dts = (int64_t)(pkt->dts*(1000 * (r2d(m_fmtCtx->streams[pkt->stream_index]->time_base))));
	return 0;
}

AVStream* Demuxer::getVideoStream()
{
	std::lock_guard<std::mutex> locker(m_mutex);
	return m_videoStream;
}

AVStream* Demuxer::getAudioStream()
{
	std::lock_guard<std::mutex> locker(m_mutex);
	return m_audioStream;
}