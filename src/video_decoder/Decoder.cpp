#include "Decoder.h"
#include "log.h"
#include <thread>

Decoder::Decoder()
	: m_codecCtx(NULL)
{

}

Decoder::~Decoder()
{

}

int Decoder::open(AVStream* stream)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_codecCtx != NULL)
	{
		LOG("codec was opened.");
		return -1;
	}

	AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (!codec)
	{
		LOG("decoder(%d) not found.", (int)stream->codecpar->codec_id);
		return -1;
	}

	m_codecCtx = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(m_codecCtx, stream->codecpar);

	// 低延时解码
	m_codecCtx->flags |= AV_CODEC_FLAG_LOW_DELAY;  

	// 多线程解码
	m_codecCtx->thread_count = 8;

	// 帧内多线程解码
	//m_codecCtx->flags |= AV_CODEC_CAP_SLICE_THREADS;
	//m_codecCtx->thread_type |= FF_THREAD_SLICE; 

	// 帧间多线程解码
	//m_codecCtx->flags |= AV_CODEC_CAP_FRAME_THREADS;
	//m_codecCtx->thread_type |= FF_THREAD_FRAME;

	m_codecCtx->flags |= AV_CODEC_FLAG_OUTPUT_CORRUPT; // allow display of corrupt frames and frames missing references
	m_codecCtx->flags2 |= AV_CODEC_FLAG2_SHOW_ALL;

	int ret = avcodec_open2(m_codecCtx, 0, 0);
	if (ret != 0)
	{
		AV_LOG(ret, "open decoder(%d) failed.", (int)stream->codecpar->codec_id);
		avcodec_free_context(&m_codecCtx);
		m_codecCtx = NULL;	
		return -1;
	}

	return 0;
}

int Decoder::close()
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_codecCtx != NULL)
	{
		avcodec_close(m_codecCtx);
		avcodec_free_context(&m_codecCtx);
		m_codecCtx = NULL;
	}

	return 0;
}

int Decoder::send(AVPacketPtr& packet)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_codecCtx == NULL)
	{
		return -1;
	}

	int ret = avcodec_send_packet(m_codecCtx, packet.get());
	if (ret != 0)
	{
		AV_LOG(ret, "send packet failed.");
		return -1;
	}

	return 0;
}

int Decoder::recv(AVFramePtr& frame)
{
	std::lock_guard<std::mutex> locker(m_mutex);

	if (m_codecCtx == NULL)
	{
		return -1;
	}

	frame.reset(av_frame_alloc(), [](AVFrame *ptr) { av_frame_free(&ptr); });
	int ret = avcodec_receive_frame(m_codecCtx, frame.get());
	if (ret != 0)
	{
		//AV_LOG(ret, "receive frame failed.");
		frame.reset();
		return -1;
	}

	return 0;
}