#pragma once

#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include <cstdio>
#include <cstdint>
#include <mutex>
#include <thread>


class WASAPIPlayer
{
public:
	WASAPIPlayer();
	~WASAPIPlayer();
	WASAPIPlayer &operator=(const WASAPIPlayer &) = delete;
	WASAPIPlayer(const WASAPIPlayer &) = delete;

	int init();
	int exit();
	int start();
	int stop();

private:
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);


};

