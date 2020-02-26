// PHZ
// 2019-11-11

#ifndef FFMPEG_DEMUXER_H
#define FFMPEG_DEMUXER_H

#include <string>
#include <mutex>
#include <memory>

extern "C" {
#include "libavformat/avformat.h"
}

typedef std::shared_ptr<AVPacket> AVPacketPtr;

class Demuxer
{
public:
	Demuxer &operator=(const Demuxer &) = delete;
	Demuxer(const Demuxer &) = delete;
	Demuxer();
	~Demuxer();

	int open(std::string url);
	int close();
	int read(AVPacketPtr& pkt);
	
	AVStream* getVideoStream();
	AVStream* getAudioStream();

private:
	std::mutex m_mutex;
	std::string m_url;

	AVFormatContext *m_fmtCtx = NULL;
	AVStream *m_videoStream = NULL;
	AVStream *m_audioStream = NULL;
	int m_videoIndex = -1;
	int m_audioIndex = -1;
};

#endif

