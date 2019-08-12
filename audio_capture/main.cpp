#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include "OpusCodec.h"

int main(int argc, char **argv)
{
	opus::OpusConfig config;
	config.samplerate = 48000;
	config.channels = 2;

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

	int frameSize = config.samplerate / 50;
	int16_t data[10000] = { 0 };
	uint8_t encBuf[10000] = { 0 };
	int16_t decBuf[10000] = { 0 };

	while (1)
	{
		int ret = fread(data, sizeof(short)*config.channels, frameSize, infile);
		if (ret == frameSize)
		{
			int len = opusEncoder.encode(data, ret, encBuf, 10000);
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
	getchar();
	return 0;
}