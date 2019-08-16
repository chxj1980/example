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

	bool curlost = false;
	int outputSamples = 0;
	int maxFrameSize = kMaxPacketSize;

	if (data == NULL || dataSize == 0)
	{
		curlost = true;
	}

	if (m_preLost && m_config.use_inbandfec)
	{
		opus_decoder_ctl(m_decoder, OPUS_GET_LAST_PACKET_DURATION(&maxFrameSize));
	}

	int curFrameSize = maxOutBufSize / m_config.channels / 2;
	if (maxFrameSize > curFrameSize)
	{
		maxFrameSize = curFrameSize;
	}

	if (m_count >= m_config.use_inbandfec) 
	{
		if (m_config.use_inbandfec)
		{
			if (m_preLost)
			{
				outputSamples = opus_decode(m_decoder, curlost ? NULL : data, dataSize, outBuf, maxFrameSize, 1);
			}
			else
			{
				if (m_prevBuffer != nullptr && m_prevBufferSize > 0)
				{
					outputSamples = opus_decode(m_decoder, m_prevBuffer.get(), m_prevBufferSize, outBuf, maxFrameSize, 0);
					m_prevBuffer = nullptr;
					maxFrameSize = 0;
				}
			}
		}
		else
		{
			outputSamples = opus_decode(m_decoder, curlost ? NULL : data, dataSize, outBuf, maxFrameSize, 0);
		}
	}

	if (m_config.use_inbandfec)
	{
		m_preLost = curlost;
		if (!m_preLost && dataSize > 0)
		{
			m_count++;
			m_prevBuffer.reset(new uint8_t[dataSize]);
			memcpy(m_prevBuffer.get(), data, dataSize);
			m_prevBufferSize = dataSize;
		}
	}

	return outputSamples;
}
}
