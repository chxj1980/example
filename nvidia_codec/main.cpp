#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "DXGIScreenCapture.h"
#include "NvEncoderD3D11.h" // Video Codec SDK 8.2
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream> 

static int frame_count = 1000;

int main(int argc, char **argv)
{
	DXGIScreenCapture screenCapture;
	if (screenCapture.init() < 0)
	{
		getchar();
		return 0;
	}
	screenCapture.start();

	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<IDXGIAdapter> pAdapter;
	Microsoft::WRL::ComPtr<IDXGIFactory1> pFactory;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexSysMem;

	HRESULT hr = S_OK;

	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void **)pFactory.GetAddressOf());
	for (int gpuIndex = 0; gpuIndex <= 1; gpuIndex++)
	{
		hr = pFactory->EnumAdapters(gpuIndex, pAdapter.GetAddressOf());
		if (FAILED(hr))
		{
			getchar();
			return -1;
		}
		else
		{
			char szDesc[128] = { 0 };
			DXGI_ADAPTER_DESC adapterDesc;
			pAdapter->GetDesc(&adapterDesc);
			wcstombs(szDesc, adapterDesc.Description, sizeof(szDesc));
			if (strstr(szDesc, "NVIDIA") == NULL)
			{
				continue;
			}
		}

		hr = D3D11CreateDevice(pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
			pDevice.GetAddressOf(), NULL, pContext.GetAddressOf());
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	
	if (pAdapter.Get() == nullptr)
	{
		printf("Failed to create d3d11 device. \n");
		return -1;
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = screenCapture.getWidth();
	desc.Height = screenCapture.getHeight();
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = pDevice->CreateTexture2D(&desc, NULL, pTexSysMem.GetAddressOf());
	if (FAILED(hr))
	{
		printf("Failed to create texture. \n");
		return -1;
	}

	NvEncoderD3D11 enc(pDevice.Get(), desc.Width, desc.Height, NV_ENC_BUFFER_FORMAT_ARGB);

	NV_ENC_INITIALIZE_PARAMS initializeParams = { NV_ENC_INITIALIZE_PARAMS_VER };
	NV_ENC_CONFIG encodeConfig = { NV_ENC_CONFIG_VER };
	initializeParams.encodeConfig = &encodeConfig;
	initializeParams.maxEncodeWidth = desc.Width;
	initializeParams.maxEncodeHeight = desc.Height;
	enc.CreateDefaultEncoderParams(&initializeParams, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_LOW_LATENCY_DEFAULT_GUID);
	enc.CreateEncoder(&initializeParams);

	std::ofstream fpOut("out.h264", std::ios::out | std::ios::binary);
	if (!fpOut)
	{
		printf("open %s failed.\n", "out.h264");
	}

	printf("start encode h264 ...\n");

	while (frame_count--)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
		std::vector<std::vector<uint8_t>> vPacket;

		{
#if 0
			uint32_t frameSize = 0;
			std::shared_ptr<uint8_t> pFrame;
			if (screenCapture.captureFrame(pFrame, frameSize) != 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}

			const NvEncInputFrame* encoderInputFrame = enc.GetNextInputFrame();
			screenCapture.captureFrame(pFrame, frameSize);
			D3D11_MAPPED_SUBRESOURCE map;
			pContext->Map(pTexSysMem.Get(), D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE, 0, &map);
			memcpy((uint8_t *)map.pData, pFrame.get(), frameSize);
			pContext->Unmap(pTexSysMem.Get(), D3D11CalcSubresource(0, 0, 1));
#else
			if (screenCapture.captureFrame(pDevice.Get(), pTexSysMem.Get()) != 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
#endif
			{
				const NvEncInputFrame* encoderInputFrame = enc.GetNextInputFrame();
				ID3D11Texture2D *pTexBgra = reinterpret_cast<ID3D11Texture2D*>(encoderInputFrame->inputPtr);
				pContext->CopyResource(pTexBgra, pTexSysMem.Get());
				enc.EncodeFrame(vPacket);
			}
		}

		for (std::vector<uint8_t> &packet : vPacket)
		{
			fpOut.write(reinterpret_cast<char*>(packet.data()), packet.size());
		}
	}

	enc.DestroyEncoder();
	screenCapture.stop();
	fpOut.close();
	printf("stop encode ...\n");
	getchar();
	return 0;
}

