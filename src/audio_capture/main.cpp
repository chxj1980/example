#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include "WASAPICapture.h"
#include "WASAPIPlayer.h"

extern int test_opus_codec();

void captureCallback(const WAVEFORMATEX *m_mixFormat, uint8_t *data, uint32_t samples)
{
	//printf("capture samples: %d\n", samples);

	static FILE* pFile = NULL;
	if (pFile == NULL) 
	{
		pFile = fopen("capture.pcm", "wb");
	}
	else 
	{
		fwrite(data, 1, m_mixFormat->nBlockAlign * samples, pFile);
	}
}

void playCallback(const WAVEFORMATEX *m_mixFormat, uint8_t *data, uint32_t samples)
{
	static FILE* pFile = NULL;
	if (pFile == NULL)
	{
		pFile = fopen("capture.pcm", "rb");
	}
	else
	{
		//printf("play samples: %d\n", samples);
		fread(data, 1, m_mixFormat->nBlockAlign * samples, pFile);
	}
}

int main(int argc, char **argv)
{
	WASAPICapture audioCapture;
	audioCapture.init();
	audioCapture.setCallback(captureCallback);
	audioCapture.start();

	//WASAPIPlayer audioPlayer;
	//audioPlayer.init();
	//audioPlayer.start(playCallback);

	while (1)
	{
		Sleep(1000);
	}

	audioCapture.stop();
	//audioPlayer.stop();
	getchar();
	return 0;
}
