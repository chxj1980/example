#include <iostream>
#include "WASAPICapture.h"

extern int test_opus_codec();

int main(int argc, char **argv)
{
	WASAPICapture audioCapture;
	audioCapture.init();

	getchar();
	return 0;
}