#include "device_manager.h"

//#include <windows.h>
//#include <hidsdi.h>

#define MAX_LOADSTRING 100

std::string wStringToString(const wchar_t* wstr, int size);
std::string getDeviceName(HANDLE hDevice);
void getDeviceInfo(PRAWINPUTDEVICELIST pRawInputDeviceList, int index, RID_DEVICE_INFO* deviceInfo);
void getDevicePreparsedData(PRAWINPUTDEVICELIST pRawInputDeviceList, int index, PHIDP_PREPARSED_DATA devicePreparsedData);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void DeviceManager::_bind_methods() {
	godot::ClassDB::bind_method(godot::D_METHOD("get_device_list"), &DeviceManager::get_device_list);
	godot::ClassDB::bind_method(godot::D_METHOD("set_window", "window_handle"), &DeviceManager::set_window);
	godot::ClassDB::bind_method(godot::D_METHOD("register_touchpads"), &DeviceManager::register_touchpads);
}

std::string wStringToString(const wchar_t* wstr, int size)
{
	std::string str = "";
	//str.resize(size);
	for (int i = 0; i < size; i++)
	{
		str = str + (char)wstr[i];
	}
	//wcstombs_s(&size, &str[0], str.size() + 1, wstr.c_str(), wstr.size());
	return str;
}

void getDevicePreparsedData(PRAWINPUTDEVICELIST pRawInputDeviceList, int index, PHIDP_PREPARSED_DATA devicePreparsedData) {
	HANDLE hDevice = pRawInputDeviceList[index].hDevice;
	UINT uiCommand = RIDI_PREPARSEDDATA;
	UINT pcbSize = 0;
	int bytesRead = GetRawInputDeviceInfoW(hDevice, uiCommand, NULL, &pcbSize);

	if (pcbSize > 0) {
		bytesRead = GetRawInputDeviceInfoW(hDevice, uiCommand, devicePreparsedData, &pcbSize);
		//std::cout << "Bytes read: " + std::to_string((int)bytesRead) << std::endl;
		if (bytesRead == (UINT)-1)
		{
			print_line(vformat("Error code: ", (int)GetLastError()));
		}
	}
}

void getDeviceInfo(PRAWINPUTDEVICELIST pRawInputDeviceList, int index, RID_DEVICE_INFO* deviceInfo) {
	HANDLE hDevice = pRawInputDeviceList[index].hDevice;
	UINT uiCommand = RIDI_DEVICEINFO;
	UINT pcbSize = 0;
	int bytesRead = GetRawInputDeviceInfoW(hDevice, uiCommand, NULL, &pcbSize);
	/* if (bytesRead == (UINT)-1)
	{
		return "";
	} */

	//std::cout << "Name size (in characters): " + std::to_string((int)pcbSize) << std::endl;

	// Using a wide character array here is extremely important, Win32 W functions use UTF-16 strings (kinda)

	if (pcbSize > 0) {
		bytesRead = GetRawInputDeviceInfoW(hDevice, uiCommand, deviceInfo, &pcbSize);
		//std::cout << "Bytes read: " + std::to_string((int)bytesRead) << std::endl;
		if (bytesRead == (UINT)-1)
		{
			print_line(vformat("Error code: %d", (int)GetLastError()));
		}
	}
}

std::string getDeviceName(HANDLE hDevice) {
	UINT uiCommand = RIDI_DEVICENAME;
	UINT pcbSize = 0;
	int bytesRead = GetRawInputDeviceInfoW(hDevice, uiCommand, NULL, &pcbSize);
	if (bytesRead == (UINT)-1)
	{
		return "";
	}

	//std::cout << "Name size (in characters): " + std::to_string((int)pcbSize) << std::endl;

	// Using a wide character array here is extremely important, Win32 W functions use UTF-16 strings (kinda)
	wchar_t* szDeviceName = new wchar_t[pcbSize];

	if (pcbSize > 0) {
		bytesRead = GetRawInputDeviceInfoW(hDevice, uiCommand, szDeviceName, &pcbSize);
		//std::cout << "Bytes read: " + std::to_string((int)bytesRead) << std::endl;
		if (bytesRead == (UINT)-1)
		{
			print_line(vformat("Error code: ", (int)GetLastError()));
		}
	}

	std::string deviceName = wStringToString(szDeviceName, pcbSize);
	delete[] szDeviceName;
	return deviceName;
}

int DeviceManager::set_window(int window_handle) {
	DeviceManager::windowHandle = (HWND)IntToPtr(window_handle);

	if (RegisterTouchWindow(DeviceManager::windowHandle, TWF_FINETOUCH | TWF_WANTPALM) == 0) {
		print_line(vformat("Touch window registration failed. Error code: %d", (int)GetLastError()));
	}

	HINSTANCE hInstance = GetModuleHandleW(NULL);

	WNDCLASSEXW wcex;
	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = L"godot cpp template (DEBUG)";
    wcex.hIconSm        = NULL;

	if (RegisterClassExW(&wcex) == 0) {
		print_line(vformat("Class registration failed. Error code: %d", (int)GetLastError()));
	}

	return 0;
}

int DeviceManager::register_touchpads() {
	if (DeviceManager::windowHandle < 0) {
		return -1;
	}
	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = HID_USAGE_PAGE_DIGITIZER;
	Rid[0].usUsage = HID_USAGE_DIGITIZER_TOUCH_PAD;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = DeviceManager::windowHandle;

	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE)
	{
		//registration failed. Call GetLastError for the cause of the error.
		print_line(vformat("Error code: ", (int)GetLastError()));
	}
	return 0;
}

godot::Array DeviceManager::get_device_list() {
	UINT numDevices = 0;
	godot::Array deviceList = godot::Array();
	PRAWINPUTDEVICELIST pRawInputDeviceList = NULL;
	//RAWINPUTDEVICELIST pRawInputDeviceList[16];
	while (true)
	{
		// Display error if GetRawInputDevice returns anything but zero (size of NULL argument's "array")
		if (GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST)) != 0)
		{
			print_line("Error: GetRawInputDeviceList failed");
		};

		// Stop if there aren't any devices
		if (numDevices == 0) break;

		pRawInputDeviceList = new RAWINPUTDEVICELIST[numDevices];
		numDevices = GetRawInputDeviceList(pRawInputDeviceList, &numDevices, sizeof(RAWINPUTDEVICELIST));
		if (GetLastError() == ERROR_INVALID_HANDLE) {
			print_line("Error: Invalid handle");
		}
		if (numDevices == (UINT)-1) {
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				print_line("Error: Insufficient buffer");
			}
			// Devices were added.
			delete[] pRawInputDeviceList;
			continue;
		}
		break;
	}
	for (UINT i = 0; i < numDevices; i++)
	{
		HANDLE hDevice = pRawInputDeviceList[i].hDevice;
		std::string deviceName = getDeviceName(hDevice);
		print_line("Current device name: " + godot::String(deviceName.c_str()));

		// Skip non-HID devices (mice, keyboards)
		if (pRawInputDeviceList[i].dwType != RIM_TYPEHID) {
			continue;
		}
		RID_DEVICE_INFO deviceInfo = RID_DEVICE_INFO();
		getDeviceInfo(pRawInputDeviceList, i, &deviceInfo);
		print_line(vformat("Current device usage page: %d", (int)deviceInfo.hid.usUsagePage));
		print_line(vformat("Current device usage: %d", (int)deviceInfo.hid.usUsage));
		// Skip non-digitizer non-touch pad devices
		if (deviceInfo.hid.usUsagePage != HID_USAGE_PAGE_DIGITIZER || deviceInfo.hid.usUsage != HID_USAGE_DIGITIZER_TOUCH_PAD) {
			continue;
		}
		// If you've made it this far, you're a trackpad
		print_line("This is a trackpad!");

		PHIDP_PREPARSED_DATA devicePreparsedData = PHIDP_PREPARSED_DATA();
		getDevicePreparsedData(pRawInputDeviceList, i, devicePreparsedData);
		HIDP_CAPS caps = HIDP_CAPS();
		HidP_GetCaps(devicePreparsedData, &caps);
		print_line(vformat("Trackpad NumberInputButtonCaps: %d", (int)caps.NumberInputButtonCaps));
	}
	delete[] pRawInputDeviceList;
	return deviceList;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	print_line("Messsage!");
	switch(uMsg)
	{
		case WM_INPUT:
			print_line(vformat("Message recieved: %d", (int)uMsg));

			HRAWINPUT hRawInput = (HRAWINPUT)lParam;

			// retrieve and process data from hRawInput as needed...

			UINT dwSize;

			GetRawInputData(hRawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			LPBYTE lpb = new BYTE[dwSize];

			if (GetRawInputData(hRawInput, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
				print_line("Wrong buffer size!");
			}

			RAWINPUT* raw = (RAWINPUT*)lpb;

			//raw->header

			std::string deviceName = getDeviceName(raw->header.hDevice);
			print_line(vformat("Device Name: " +  godot::String(deviceName.c_str())));

			delete[] lpb;
			return 0;
		/* else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}	 */
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}