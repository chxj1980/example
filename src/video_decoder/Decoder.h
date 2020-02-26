// PHZ
// 2019-11-11

#ifndef FFMPEG_DECODER_H
#define FFMPEG_DECODER_H

#include <string>
#include <mutex>
#include <memory>
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

typedef std::shared_ptr<AVFrame> AVFramePtr;

class Decoder
{
public:
	typedef std::shared_ptr<AVPacket> AVPacketPtr;

	Decoder &operator=(const Decoder &) = delete;
	Decoder(const Decoder &) = delete;
	Decoder();
	virtual ~Decoder();

	virtual int open(AVStream* stream);
	virtual int close();

	virtual int send(AVPacketPtr& packet);
	virtual int recv(AVFramePtr& frame);

	AVCodecContext *getCodecContext()
	{ return m_codecCtx; }

private:
	std::mutex m_mutex;
	AVCodecContext *m_codecCtx = NULL;
};

#endif
