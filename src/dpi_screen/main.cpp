#include <cstdio>
#include <iostream>
#include <Windows.h>
#include <versionhelpers.h>
#include <shellscalingapi.h>

/* 获取屏幕缩放百分比 */
double GetScreenScale()
{
	double scale = 1.0;

	HWND hwnd = NULL;
	int cxLogical = 0, cyLogical = 0;
	int cxPhysical = 0, cyPhysical = 0;
	MONITORINFOEX miex;
	DEVMODE dm;

	hwnd = GetDesktopWindow();
	if (hwnd == NULL)
	{
		return scale;
	}

	HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	if (hMonitor == NULL)
	{
		return scale;
	}

	miex.cbSize = sizeof(miex);
	GetMonitorInfo(hMonitor, &miex);
	cxLogical = (miex.rcMonitor.right - miex.rcMonitor.left);
	cyLogical = (miex.rcMonitor.bottom - miex.rcMonitor.top);

	dm.dmSize = sizeof(dm);
	dm.dmDriverExtra = 0;
	EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm);
	cxPhysical = dm.dmPelsWidth;
	cyPhysical = dm.dmPelsHeight;

	if (cxLogical != 0 && cyLogical != 0)
	{
		scale = ((double)cxPhysical / (double)cxLogical);
		//scale = ((double)cyPhysical / (double)cyLogical);
	}

	return scale;
}

typedef HRESULT(*SetProcessDpiAwareness_Func) (PROCESS_DPI_AWARENESS value);
typedef BOOL(*SetProcessDPIAware_Func) ();

int SetProcessDpiAwareness()
{
#ifdef WIN32
	HRESULT hr;
	if (IsWindows8OrGreater()) /* win8, win10 */
	{
		HMODULE hModule = ::GetModuleHandle(TEXT("Shcore.dll"));
		if (hModule != NULL)
		{
			SetProcessDpiAwareness_Func pFunc = (SetProcessDpiAwareness_Func)GetProcAddress(hModule, "SetProcessDpiAwareness");
			if (pFunc != NULL)
			{
				hr = pFunc(PROCESS_PER_MONITOR_DPI_AWARE);
				if (FAILED(hr))
				{
					fprintf(stderr, "SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) failed.\n");
					return -1;
				}
			}
		}
		else
		{
			fprintf(stderr, "Shcore.dll not found.\n");
			return -1;
		}
	}
	else /* win7 */
	{
		HMODULE hModule = ::GetModuleHandle(TEXT("User32.dll"));
		if (hModule != NULL)
		{
			SetProcessDPIAware_Func pFunc = (SetProcessDPIAware_Func)GetProcAddress(hModule, "SetProcessDPIAware");
			if (pFunc != NULL)
			{
				pFunc();
			}
		}
		else
		{
			fprintf(stderr, "User32.dll not found.\n");
			return -1;
		}
	}
#endif
	return 0;
}

int main(int argc, char **argv)
{
	std::cout << "screen scale: " << GetScreenScale() << std::endl;
	getchar();
	return 0;
}