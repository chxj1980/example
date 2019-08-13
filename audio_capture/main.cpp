#include <iostream>
#include "WASAPICapture.h"

extern int test_opus_codec();

void dataCB(WAVEFORMATEX *m_mixFormat, BYTE *data, uint32_t samples)
{
	printf("audio samples: %d\n", samples);
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