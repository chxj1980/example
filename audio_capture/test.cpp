#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "OpusCodec.h"

int test_opus_codec()
{
	opus::OpusConfig config;
	config.samplerate = 48000;
	config.channels = 2;
	config.bitrate = 32000;
	config.variable_duration = OPUS_FRAMESIZE_20_MS;

	opus::OpusEncoder opusEncoder;
	opusEncoder.init(config);

	opus::OpusDecoder opusDecoder;
	opusDecoder.init(config);

	FILE *infile = fopen("test.pcm", "rb");
	if (infile == NULL)
	{
		return -1;
	}

	FILE *outfile = fopen("out.pcm", "wb");
	if (outfile == NULL)
	{
		return -1;
	}

	int frameSize = config.samplerate / 50; /* 20 ms*/
	int16_t data[10000] = { 0 };
	uint8_t encBuf[10000] = { 0 };
	int16_t decBuf[10000] = { 0 };

	while (1)
	{
		size_t ret = fread(data, sizeof(short)*config.channels, frameSize, infile);
		if (ret == frameSize)
		{
			int len = opusEncoder.encode(data, (int)ret, encBuf, 10000);
			if (len > 0)
			{
				int samples = opusDecoder.decode(encBuf, len, decBuf, 10000);
				fwrite(decBuf, sizeof(short)*config.channels, samples, outfile);
			}
		}
		else
		{
			break;
		}
	}

	fclose(outfile);
	fclose(infile);
	return 0;
}