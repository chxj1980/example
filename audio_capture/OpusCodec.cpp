#include "OpusCodec.h"

namespace opus
{

OpusEncoder::OpusEncoder()
{

}

OpusEncoder::~OpusEncoder()
{

}

int OpusEncoder::init(OpusConfig config)
{
	if (m_initialized)
	{
		return -1;
	}

	int error = 0;
	m_encoder = opus_encoder_create(config.samplerate, config.channels, config.application, &error);
	if (error!=OPUS_OK || m_encoder==NULL)
	{
		fprintf(stderr, "[OpusEncoder] Cannot create encoder: %s\n", opus_strerror(error));
		return -1;
	}

	opus_encoder_ctl(m_encoder, OPUS_SET_BITRATE(config.bitrate));
	opus_encoder_ctl(m_encoder, OPUS_SET_BANDWIDTH(config.bandwidth));
	opus_encoder_ctl(m_encoder, OPUS_SET_VBR(config.use_vbr));
	opus_encoder_ctl(m_encoder, OPUS_SET_VBR_CONSTRAINT(config.use_cvbr));
	opus_encoder_ctl(m_encoder, OPUS_SET_COMPLEXITY(config.complexity));
	opus_encoder_ctl(m_encoder, OPUS_SET_INBAND_FEC(config.use_inbandfec));
	opus_encoder_ctl(m_encoder, OPUS_SET_FORCE_CHANNELS(config.forcechannels));
	opus_encoder_ctl(m_encoder, OPUS_SET_DTX(config.use_dtx));
	opus_encoder_ctl(m_encoder, OPUS_SET_PACKET_LOSS_PERC(config.packet_loss_perc));
	opus_encoder_ctl(m_encoder, OPUS_GET_LOOKAHEAD(&config.skip));
	opus_encoder_ctl(m_encoder, OPUS_SET_LSB_DEPTH(16));
	opus_encoder_ctl(m_encoder, OPUS_SET_EXPERT_FRAME_DURATION(config.variable_duration));

	memcpy(&m_config, &config, sizeof(m_config));
	m_initialized = true;
	return 0;
}

int OpusEncoder::exit()
{
	if (!m_initialized)
	{
		return -1;
	}

	opus_encoder_destroy(m_encoder);
	m_initialized = false;
	return 0;
}

int OpusEncoder::encode(int16_t *pcm, int frameSize, uint8_t *outBuf, int maxOutBufSize)
{
	if (!m_initialized)
	{
		return -1;
	}

	int max_payload_bytes = maxOutBufSize < 1500 ? maxOutBufSize : 1500;
	int len = opus_encode(m_encoder, pcm, frameSize, outBuf, maxOutBufSize);

	int bytes = opus_packet_get_samples_per_frame(outBuf, m_config.samplerate)*opus_packet_get_nb_frames(outBuf, len);
	//printf("len: %d, bytes:%d\n", len, bytes);

	return len;
}

OpusDecoder::OpusDecoder()
{

}

OpusDecoder::~OpusDecoder()
{

}

int OpusDecoder::init(OpusConfig config)
{
	if (m_initialized)
	{
		return -1;
	}

	int error = 0;
	m_decoder = opus_decoder_create(config.samplerate, config.channels, &error);
	if (error != OPUS_OK || m_decoder == NULL)
	{
		fprintf(stderr, "[OpusDecoder] Cannot create encoder: %s\n", opus_strerror(error));
		return -1;
	}

	m_initialized = true;
	return 0;
}

int OpusDecoder::exit()
{
	if (!m_initialized)
	{
		return -1;
	}

	opus_decoder_destroy(m_decoder);
	m_initialized = false;
	return 0;
}

int OpusDecoder::decode(uint8_t *data, int dataSize, int16_t *outBuf, int maxOutBufSize)
{
	if (!m_initialized)
	{
		return -1;
	}

	int maxFrameSize = 48000 * 2 * 2;
	if (maxFrameSize > maxOutBufSize)
	{
		maxFrameSize = maxOutBufSize;
	}

	int outputSamples = opus_decode(m_decoder, data, dataSize, outBuf, maxFrameSize, 0);
	//printf("outputSamples: %d\n", outputSamples);

	return outputSamples;
}

}
