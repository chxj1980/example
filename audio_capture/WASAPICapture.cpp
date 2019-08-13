#include "WASAPICapture.h"

WASAPICapture::WASAPICapture()
	: m_initialized(false)
	, m_mixFormat(NULL)
{

}

WASAPICapture::~WASAPICapture()
{
	CoUninitialize();
}

int WASAPICapture::init()
{
	if (m_initialized)
	{
		return 0;
	}

	CoInitialize(NULL);

	HRESULT hr = S_OK;
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)m_enumerator.GetAddressOf());
	if (FAILED(hr)) 
	{
		printf("[WASAPICapture] Failed to create instance.\n");
		return -1;
	}

	hr = m_enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, m_device.GetAddressOf());
	if (FAILED(hr)) 
	{
		printf("[WASAPICapture] Failed to create device.\n");
		return -1;
	}

	hr = m_device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)m_audioClient.GetAddressOf());
	if (FAILED(hr)) 
	{
		printf("[WASAPICapture] Failed to activate device.\n");
		return -1;
	}

	hr = m_audioClient->GetMixFormat(&m_mixFormat);
	if (FAILED(hr)) 
	{
		printf("[WASAPICapture] Failed to get mix format.\n");
		return -1;
	}

	m_initialized = true;
	return 0;
}

int WASAPICapture::exit()
{
	return 0;
}

int WASAPICapture::start()
{
	return 0;
}

int WASAPICapture::stop()
{
	return 0;
}