#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "AudioMixer.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")

int main(int argc, char **argv)
{

	AudioMixer amix;
	amix.addAudioInput(0, 48000, 2, 16);
	//amix.addAudioInput(0, 44100, 2, 32);
	amix.addAudioInput(1, 48000, 2, 16);
	//amix.addAudioInput(2, 48000, 2, 32);
	amix.addAudioOutput(44100, 2, 32);

	if (amix.init() < 0)
	{
		return -1;
	}

	int len1 = 0, len2 = 0;
	uint8_t buf1[4096], buf2[4096];
	//FILE *file1 = fopen("coreaudio_48000_2_16.pcm", "rb"); 
	//FILE *file1 = fopen("output_44100_2_32.pcm", "rb"); 
	FILE *file1 = fopen("coreaudio.pcm", "rb");
	FILE *file2 = fopen("dsound_48000_2_16.pcm", "rb");

	uint8_t outBuf[10240];

	while (1)
	{
		len1 = fread(buf1, 1, 2048, file1);
		len2 = fread(buf2, 1, 2048, file2);
		if (len1 > 0 || len2 > 0)
		{
			if (len1 > 0)
			{
				amix.addFrame(0, buf1, len1);
			}
			
			if (len2 > 0)
			{
				amix.addFrame(1, buf2, len2);
			}

			int ret = amix.getFrame(outBuf, 10240);
			printf("mix audio: %d\n", ret);
			if (ret > 0)
			{
				static FILE* pFile = NULL;
				if (pFile == NULL) 
				{
					pFile = fopen("output.pcm", "wb");
				}
				else 
				{
					fwrite(outBuf, 1, ret, pFile);
				}
			}			
		}
		else
		{
			break;
		}
	}

	getchar();
	return 0;
}

