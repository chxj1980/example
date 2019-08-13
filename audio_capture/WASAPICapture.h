// https://docs.microsoft.com/en-us/previous-versions//ms678709(v=vs.85)

#ifndef WASAPI_CAPTURE_H
#define WASAPI_CAPTURE_H

#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include <cstdio>
#include <cstdint>
#include <mutex>
#include <thread>

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

class WASAPICapture
{
public:
	WASAPICapture();
	~WASAPICapture();
	WASAPICapture &operator=(const WASAPICapture &) = delete;
	WASAPICapture(const WASAPICapture &) = delete;

	int init();
	int exit();
	int start();
	int stop();

private:
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	const IID IID_IAudioClient = __uuidof(IAudioClient);
	const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

	bool m_initialized;
	WAVEFORMATEX *m_mixFormat;
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> m_enumerator;
	Microsoft::WRL::ComPtr<IMMDevice> m_device;
	Microsoft::WRL::ComPtr<IAudioClient> m_audioClient;
	Microsoft::WRL::ComPtr<IAudioCaptureClient> m_audioCaptureClient;
};

#endif