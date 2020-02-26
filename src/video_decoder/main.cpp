#include "Demuxer.h"
#include "Decoder.h"
#include "Timestamp.h"
#include <Windows.h>

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")

int main(int argc, char **argv)
{
	Demuxer demuxer;
	Decoder decoder;

	int fps = 0;
	Timestamp timestamp;

	//int ret = demuxer.open("F:\\video\\piper.mp4");
	int ret = demuxer.open("test.h264");
	AVStream* videoStream = demuxer.getVideoStream();
	if (videoStream)
	{
		decoder.open(videoStream);
	}

	timestamp.reset();
	int seq = 0;
	while (videoStream)
	{
		AVPacketPtr packetPtr = nullptr;
		AVFramePtr  framePtr = nullptr;

		if (demuxer.read(packetPtr) == 0) 
		{
			if (packetPtr != nullptr && packetPtr->stream_index == videoStream->index)
			{
				seq++;
				if (decoder.send(packetPtr) != 0)
				{
					break;
				}

				if (!decoder.recv(framePtr))
				{
					if (framePtr && framePtr->pkt_size > 0)
					{
						fps++;
						if (fps == 1)
						{
							printf("first frame, seq = %d\n", seq);
						}

						if (fps == 500)
						{
							break;
						}
					}
				}
			}
		}
		else
		{
			break;
		}

		Sleep(1);
	}

	printf("\nfps: %d, time: %lld \n", fps, timestamp.elapsed());
	getchar();
	return 0;
}