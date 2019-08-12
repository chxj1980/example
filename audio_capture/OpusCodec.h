#pragma once

#include "opus.h"
#include <cstdio>
#include <cstdint>
#include <cstring>

namespace opus
{

struct OpusConfig
{
	int samplerate = 48000;
	int channels = 2;
	int application = OPUS_APPLICATION_VOIP;
	int bitrate = 16000 * 2; //bps
	int bandwidth = OPUS_AUTO;
	int use_vbr = 1;
	int use_cbr = 0;
	int use_cvbr = 0;
	int complexity = 10;
	int use_inbandfec = 0;
	int forcechannels = OPUS_AUTO;
	int use_dtx = 0;
	int packet_loss_perc = 0;
	int skip = 0;
	int variable_duration = OPUS_FRAMESIZE_20_MS;
};

class OpusEncoder
{
public:
	OpusEncoder();
	~OpusEncoder();

	int init(OpusConfig config);
	int exit();

	int encode(int16_t *pcm, int frameSize, uint8_t *outBuf, int maxOutBufSize);

private:
	bool m_initialized = false;
	OpusConfig m_config;

	::OpusEncoder *m_encoder = NULL;
};

class OpusDecoder
{
public:
	OpusDecoder();
	~OpusDecoder();

	int init(OpusConfig config);
	int exit();

	int decode(uint8_t *data, int dataSize, int16_t *outBuf, int maxOutBufSize);

private:
	bool m_initialized = false;
	OpusConfig m_config;

	::OpusDecoder *m_decoder = NULL;
};

}



