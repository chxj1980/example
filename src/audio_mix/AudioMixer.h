// PHZ
// 2019-9-29

#ifndef FFMPEG_AUDIO_MIXER_H
#define FFMPEG_AUDIO_MIXER_H

#include <map>
#include <mutex>
#include <cstdio>
#include <cstdint>
#include <string>     

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
}

class AudioMixer
{
public:
	AudioMixer();
	virtual ~AudioMixer();

	int addAudioInput(uint32_t index, uint32_t samplerate, uint32_t channels, uint32_t bitsPerSample);
	int addAudioOutput(uint32_t samplerate, uint32_t channels, uint32_t bitsPerSample);

	int init();
	int exit();

	int addFrame(uint32_t index, uint8_t *inBuf, uint32_t size);
	int getFrame(uint8_t *outBuf, uint32_t maxOutBufSize);

private:

	std::string getSampleFmtName(uint32_t bitsPerSample);
	AVSampleFormat getSampleFmt(uint32_t bitsPerSample);

	struct AudioInfo
	{
		AudioInfo()
		{
			filterCtx = nullptr;
		}

		uint32_t samplerate;
		uint32_t channels;
		uint32_t bitsPerSample;
		std::string name;

		AVFilterContext *filterCtx;
	};

	bool m_initialized = false;
	std::mutex m_mutex;
	std::map<uint32_t, AudioInfo> m_audioInputInfo;
	std::shared_ptr<AudioInfo> m_audioOutputInfo;
	std::shared_ptr<AudioInfo> m_audioMixInfo;
	std::shared_ptr<AudioInfo> m_audioSinkInfo;

	AVFilterGraph *m_filterGraph = nullptr;
};

#endif
