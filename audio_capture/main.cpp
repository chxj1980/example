#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include "WASAPICapture.h"

extern int test_opus_codec();

void dataCB(WAVEFORMATEX *m_mixFormat, uint8_t *data, uint32_t samples)
{
	printf("audio samples: %d\n", samples);

	static FILE* pFile = NULL;
	if (pFile == NULL) 
	{
		pFile = fopen("capture.pcm", "wb");
	}
	else {
		fwrite(data, 1, m_mixFormat->nBlockAlign * samples, pFile);
	}
}

int main(int argc, char **argv)
{
	WASAPICapture audioCapture;
	audioCapture.init();
	audioCapture.setCallback(dataCB);
	audioCapture.start();

	while (1)
	{
		Sleep(1000);
	}

	audioCapture.stop();
	getchar();
	return 0;
}