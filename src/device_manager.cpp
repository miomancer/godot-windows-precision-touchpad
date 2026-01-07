#include "device_manager.h"

//#include <windows.h>
//#include <hidsdi.h>

std::string wStringToString(const wchar_t* wstr, int size);
std::string getDeviceName(HANDLE hDevice);
void getDeviceInfo(PRAWINPUTDEVICELIST pRawInputDeviceList, int index, RID_DEVICE_INFO* deviceInfo);
void getDevicePreparsedData(PRAWINPUTDEVICELIST pRawInputDeviceList, int index, PHIDP_PREPARSED_DATA devicePreparsedData);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void DeviceManager::_bind_methods() {
	godot::ClassDB::bind_method(godot::D_METHOD("get_device_list"), &DeviceManager::get_device_list);
	godot::ClassDB::bind_method(godot::D_METHOD("get_touch_position", "index"), &DeviceManager::get_touch_position);
	godot::ClassDB::bind_method(godot::D_METHOD("set_window", "window_handle"), &DeviceManager::set_window);
	godot::ClassDB::bind_method(godot::D_METHOD("register_touchpads"), &DeviceManager::register_touchpads);
}

DeviceManager::DeviceManager() {
	singleton = this;
	this->touch_positions.resize(5);
	for (int i = 0; i < 5; i++) {
		this->touch_positions[i] = godot::Vector2(-1, -1);
	}
}
DeviceManager::~DeviceManager() {
	singleton = nullptr;
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


void getHidDPreparsedData(HANDLE hDevice, PHIDP_PREPARSED_DATA devicePreparsedData) {
	if (HidD_GetPreparsedData(hDevice, &devicePreparsedData) == false) {
		print_line(vformat("ERROR: Could not get HidD preparsed data. Error code: %d", (int)GetLastError()));
	}
}

void getDevicePreparsedData(HANDLE hDevice, PHIDP_PREPARSED_DATA devicePreparsedData, UINT pcbSize) {
	UINT uiCommand = RIDI_PREPARSEDDATA;

	if (pcbSize > 0) {
		int bytesRead = GetRawInputDeviceInfoW(hDevice, uiCommand, devicePreparsedData, &pcbSize);
		std::cout << "Bytes read: " + std::to_string((int)bytesRead) << std::endl;
		if (bytesRead == -1)
		{
			std::cout << "Could not get preparsed data. Error code: " + std::to_string((int)GetLastError()) << std::endl;
		}
	}
}

void getDeviceInfo(HANDLE hDevice, RID_DEVICE_INFO* deviceInfo) {
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
			print_line(vformat("ERROR: Could not get device info. Error code: %d", (int)GetLastError()));
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
			print_line(vformat("ERROR: Could not get device name. Error code: %d", (int)GetLastError()));
		}
	}

	std::string deviceName = wStringToString(szDeviceName, pcbSize);
	delete[] szDeviceName;
	return deviceName;
}

int DeviceManager::set_window(int64_t window_handle) {
	DeviceManager::windowHandle = (HWND)IntToPtr(window_handle);

	DeviceManager::origWndProc = (WNDPROC)GetWindowLongPtrW(DeviceManager::windowHandle, GWLP_WNDPROC);

	SetWindowLongPtrW(DeviceManager::windowHandle, GWLP_WNDPROC, (LONG_PTR)*WndProc);
	
	/* if (RegisterTouchWindow(DeviceManager::windowHandle, TWF_FINETOUCH | TWF_WANTPALM) == 0) {
		print_line(vformat("ERROR: Touch window registration failed. Error code: %d", (int)GetLastError()));
	}

	print_line(vformat("Max touches: %d", GetSystemMetrics(SM_MAXIMUMTOUCHES))); */

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

	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == false)
	{
		//registration failed. Call GetLastError for the cause of the error.
		print_line(vformat("ERROR: Could not register touchpads. Error code: %d", (int)GetLastError()));
	}


	/* MSG msg;
	int i = 0;
    while (GetMessage(&msg, NULL, 0, 0) > 0 && i < 100)
    {
		print_line(vformat("Message type: %d", (int)msg.message));
        TranslateMessage(&msg);
        DispatchMessage(&msg);
		i += 1;
    } */


	return 0;
}

DeviceManager *DeviceManager::singleton = nullptr;

DeviceManager* DeviceManager::get_singleton() {
	return singleton;
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
		getDeviceInfo(hDevice, &deviceInfo);
		print_line(vformat("Current device usage page: %d", (int)deviceInfo.hid.usUsagePage));
		print_line(vformat("Current device usage: %d", (int)deviceInfo.hid.usUsage));
		// Skip non-digitizer non-touch pad devices
		if (deviceInfo.hid.usUsagePage != HID_USAGE_PAGE_DIGITIZER || deviceInfo.hid.usUsage != HID_USAGE_DIGITIZER_TOUCH_PAD) {
			continue;
		}
		// If you've made it this far, you're a trackpad
		print_line("This is a trackpad!");

		UINT pcbSize = 0;
		int bytesRead = GetRawInputDeviceInfoW(hDevice, RIDI_PREPARSEDDATA, NULL, &pcbSize);
		LPBYTE preparsedDataBuffer = new BYTE[pcbSize];
		PHIDP_PREPARSED_DATA devicePreparsedData = (PHIDP_PREPARSED_DATA)preparsedDataBuffer;
		getDevicePreparsedData(hDevice, devicePreparsedData, pcbSize);

		HIDP_CAPS caps = HIDP_CAPS();
		if (HidP_GetCaps(devicePreparsedData, &caps) != HIDP_STATUS_SUCCESS) {
			print_line("The specified preparsed data is invalid. ");
		}
		
		/* ULONG tipSwitchValue = 0;
		unsigned long tipSwitchUsageValueResult = HidP_GetUsageValue(
			HidP_Input,
			HID_USAGE_PAGE_DIGITIZER,
			i,
			HID_USAGE_DIGITIZER_TIP_SWITCH,
			&tipSwitchValue,
			devicePreparsedData,
			cap,
			caps.FeatureReportByteLength
		);

		if (tipSwitchUsageValueResult == HIDP_STATUS_SUCCESS) {
			print_line(vformat("Tip Switch: %d", (unsigned int)tipSwitchValue));
		} else {
			print_line("Could not get Tip Switch.");
		}data.hid.dwSizeHid

		delete[] preparsedDataBuffer; */

		/* PHIDP_PREPARSED_DATA devicePreparsedData = PHIDP_PREPARSED_DATA();
		getDevicePreparsedData(hDevice, devicePreparsedData);
		HIDP_CAPS caps = HIDP_CAPS();
		HidP_GetCaps(devicePreparsedData, &caps);
		print_line(vformat("Trackpad NumberInputButtonCaps: %d", (int)caps.NumberInputButtonCaps)); */
	}
	delete[] pRawInputDeviceList;
	return deviceList;
}

WNDPROC DeviceManager::getOrigWndProc() {
	return DeviceManager::origWndProc;
}

godot::Vector2 DeviceManager::get_touch_position(int index) {
	return this->touch_positions[index];
}


void DeviceManager::set_touch_position(int index, int x, int y) {
	DeviceManager::touch_positions[index] = godot::Vector2(x, y);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//print_line(vformat("Message type: %d", (int)uMsg));
	switch(uMsg)
	{
		case WM_INPUT:
		{
			//print_line(vformat("Message recieved: %d", (int)uMsg));

			HRAWINPUT hRawInput = (HRAWINPUT)lParam;

			// retrieve and process data from hRawInput as needed...

			UINT dwSize;

			GetRawInputData(hRawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			LPBYTE rawReportBuffer = new BYTE[dwSize];

			if (GetRawInputData(hRawInput, RID_INPUT, rawReportBuffer, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
				print_line("Incorrect buffer size.");
			}

			RAWINPUT* raw = (RAWINPUT*)rawReportBuffer;

			//raw->header
			
			//std::string deviceName = getDeviceName(raw->header.hDevice);
			//print_line(vformat("Device Name: " +  godot::String(deviceName.c_str())));

			HANDLE hDevice = raw->header.hDevice;

			RID_DEVICE_INFO deviceInfo = RID_DEVICE_INFO();
			getDeviceInfo(hDevice, &deviceInfo);

			bool isTrackpad = false;
			if (raw->header.dwType == RIM_TYPEHID && deviceInfo.hid.usUsagePage == HID_USAGE_PAGE_DIGITIZER && deviceInfo.hid.usUsage == HID_USAGE_DIGITIZER_TOUCH_PAD) {
				isTrackpad = true;
			}
			if (isTrackpad) {
				//DWORD rawDataSize = raw->header.dwSize;
				//LPBYTE lpb = new BYTE[rawDataSize];
				//RAWINPUT* raw = (RAWINPUT*)lpb;

				//print_line(vformat("Input size: %d", (int)raw->data.hid.dwSizeHid));
				//print_line(vformat("Input count: %d", (int)raw->data.hid.dwCount));

				UINT pcbSize = 0;
				int bytesRead = GetRawInputDeviceInfoW(hDevice, RIDI_PREPARSEDDATA, NULL, &pcbSize);
				LPBYTE preparsedDataBuffer = new BYTE[pcbSize];
				PHIDP_PREPARSED_DATA devicePreparsedData = (PHIDP_PREPARSED_DATA)preparsedDataBuffer;
				getDevicePreparsedData(hDevice, devicePreparsedData, pcbSize);
				
				HIDP_CAPS caps = HIDP_CAPS();
				if (HidP_GetCaps(devicePreparsedData, &caps) != HIDP_STATUS_SUCCESS) {
					print_line("The specified preparsed data is invalid. ");
				}

				//print_line(vformat("Trackpad NumberInputButtonCaps: %d", (int)caps.NumberInputButtonCaps));
				//print_line(vformat("Trackpad NumberInputValueCaps: %d", (int)caps.NumberInputValueCaps));
				USHORT numButtonCaps = caps.NumberInputButtonCaps;
				USHORT numValueCaps = caps.NumberInputValueCaps;


				// Get Button Caps
				LPBYTE buttonCapsBuffer = new BYTE[numButtonCaps * sizeof(HIDP_BUTTON_CAPS)];
				PHIDP_BUTTON_CAPS buttonCaps = (PHIDP_BUTTON_CAPS)buttonCapsBuffer;
				
				unsigned long getButtonResult = HidP_GetButtonCaps(HidP_Input, buttonCaps, &numButtonCaps, devicePreparsedData);
				if (getButtonResult != HIDP_STATUS_SUCCESS) {
					print_line(vformat("ERROR: Get button caps failed. Error code: %d", (unsigned int)getButtonResult));
				}

				// Get Value Caps
				LPBYTE valueCapsBuffer = new BYTE[numValueCaps * sizeof(HIDP_VALUE_CAPS)];
				PHIDP_VALUE_CAPS valueCaps = (PHIDP_VALUE_CAPS)valueCapsBuffer;
				
				unsigned long getValueResult = HidP_GetValueCaps(HidP_Input, valueCaps, &numValueCaps, devicePreparsedData);
				if (getValueResult != HIDP_STATUS_SUCCESS) {
					print_line(vformat("ERROR: Get button caps failed. Error code: %d", (unsigned int)getValueResult));
				}

				//print_line(vformat("Value Caps ReportCount: %d", (unsigned int)valueCaps->ReportCount));
				
				
				LPBYTE reportBuffer = new BYTE[(int)(raw->header.dwSize)]{0};
				PCHAR report = (PCHAR)reportBuffer;
				UCHAR reportID = report[0];

				bool reportIDMatches = false;
				//print_line(vformat("Report ID: %d", (unsigned int)buttonCaps->ReportID));
				if (reportID == buttonCaps->ReportID) {
					reportIDMatches = true;
					//print_line(vformat("Report ID Match: %d", (unsigned int)buttonCaps->ReportID));
				}
				
				ULONG contactCountValue = 0;
				unsigned long contactCountUsageValueResult = HidP_GetUsageValue(
					HidP_Input,
					HID_USAGE_PAGE_DIGITIZER,
					0,
					HID_USAGE_DIGITIZER_CONTACT_COUNT,
					&contactCountValue,
					devicePreparsedData,
					(PCHAR)raw->data.hid.bRawData,
					raw->data.hid.dwSizeHid
				);
				



				//print_line(vformat("Contact Count: %d", (unsigned int)contactCountValue));

				for (int i = 0; i < 5; i++) {

					// Get range of indices returned by HIDClass driver for buttons
					ULONG buttonUsageRange = numButtonCaps;

					// Get usages
					LPBYTE usageBuffer = new BYTE[(int)(buttonUsageRange)];
					PUSAGE usageList = (PUSAGE)usageBuffer;



					unsigned long usageResult = HidP_GetUsages(
					HidP_Input,
					deviceInfo.hid.usUsagePage,
					i + 1,
					usageList,
					&buttonUsageRange,
					devicePreparsedData,
					(PCHAR)raw->data.hid.bRawData,
					raw->data.hid.dwSizeHid
					);
					switch (usageResult)
					{
					case HIDP_STATUS_INVALID_REPORT_LENGTH:
						print_line("The report length is not valid.");
						break;
					case HIDP_STATUS_INVALID_REPORT_TYPE:
						print_line("The specified report type is not valid.");
						break;
					case HIDP_STATUS_BUFFER_TOO_SMALL:
						print_line("The UsageList buffer is too small to hold all the usages that are currently set to ON on the specified usage page.");
						break;
					case HIDP_STATUS_INCOMPATIBLE_REPORT_ID:
						print_line("The collection contains buttons on the specified usage page in a report of the specified type, but there are no such usages in the specified report.");
						break;
					case HIDP_STATUS_INVALID_PREPARSED_DATA:
						print_line("The preparsed data is not valid.");
						break;
					case HIDP_STATUS_USAGE_NOT_FOUND:
						print_line("The collection does not contain any buttons on the specified usage page in any report of the specified report type.");
						break;
					default:
						break;
					}

					/* if (i == 0) {
						print_line(vformat("Button Usage Range: %d", (unsigned int)buttonUsageRange));
					}
					print_line(vformat("Button Usage Range: %d", (unsigned int)buttonUsageRange)); */
					bool hasTipSwitch = false;
					for (int i = 0; i < buttonUsageRange; i++) {
						//print_line(vformat("ON Button usage: %d", (unsigned int)usageList[i]));
						if (usageList[i] == HID_USAGE_DIGITIZER_TIP_SWITCH) {
							hasTipSwitch = true;
							break;
						}
						//print_line(vformat("Button usage: %d", (unsigned int)buttonCaps[i].NotRange.Usage));
					}
					if (!hasTipSwitch) {
						DeviceManager::get_singleton()->set_touch_position(i, -1, -1);
						delete[] usageBuffer;
						continue;
					}



					if (i < contactCountValue) {
						ULONG xValue = 0;
						unsigned long positionUsageValueResult = HidP_GetUsageValue(
							HidP_Input,
							HID_USAGE_PAGE_GENERIC,
							i + 1,
							HID_USAGE_GENERIC_X,
							&xValue,
							devicePreparsedData,
							(PCHAR)raw->data.hid.bRawData,
							raw->data.hid.dwSizeHid
						);

						/* if (positionUsageValueResult == HIDP_STATUS_SUCCESS) {
							print_line(vformat("X Usage, Usage value: %d", (unsigned int)xValue));
						} */

						ULONG yValue = 0;
						positionUsageValueResult = HidP_GetUsageValue(
							HidP_Input,
							HID_USAGE_PAGE_GENERIC,
							i + 1,
							HID_USAGE_GENERIC_Y,
							&yValue,
							devicePreparsedData,
							(PCHAR)raw->data.hid.bRawData,
							raw->data.hid.dwSizeHid
						);


						DeviceManager::get_singleton()->set_touch_position(i, (int)xValue, (int)yValue);

						/* if (positionUsageValueResult == HIDP_STATUS_SUCCESS) {
							print_line(vformat("Y Usage, Usage value: %d", (unsigned int)yValue));
						} */
					} /* else {
						DeviceManager::get_singleton()->set_touch_position(i, -1, -1);
					} */
					delete[] usageBuffer;
				}



				//DeviceManager::get_singleton()->call("print", "INPUT DETECTED");

				/* USHORT xUsageValueByteLength= valueCaps->BitSize * valueCaps->ReportCount;
				xUsageValueByteLength = (int)(valueCaps->BitSize);

				LPBYTE xUsageValueArrayBuffer = new BYTE[(int)(xUsageValueByteLength)]{0};
				PCHAR xUsageValueArray = (PCHAR)xUsageValueArrayBuffer;

				unsigned long xUsageValueArrayResult = HidP_GetUsageValueArray(
					HidP_Input,
					HID_USAGE_PAGE_GENERIC,
					0,
					HID_USAGE_GENERIC_Y,
					xUsageValueArray,
					xUsageValueByteLength,
					devicePreparsedData,
					(PCHAR)raw->data.hid.bRawData,
					raw->data.hid.dwSizeHid * raw->data.hid.dwCount
				);


				if (xUsageValueArrayResult == HIDP_STATUS_SUCCESS) {
					print_line(vformat("Test: %d", (unsigned int)xUsageValueArray[0]));
				} */











				/* ULONG value = 0;
				for(int i = 0; i < caps.NumberInputValueCaps; i++)
				{
					value = 0;
					unsigned long usageValueResult = HidP_GetUsageValue(
						HidP_Input,
						deviceInfo.hid.usUsagePage,
						0,
						valueCaps[i].Range.UsageMin,
						&value,
						devicePreparsedData,
						(PCHAR)raw->data.hid.bRawData,
						raw->data.hid.dwSizeHid * raw->data.hid.dwCount
					);
					switch (usageValueResult)
					{
					case HIDP_STATUS_INVALID_REPORT_LENGTH:
						print_line("ERROR: Could not get usage value.");
						print_line("The report length is not valid.");
						break;
					case HIDP_STATUS_INVALID_REPORT_TYPE:
						print_line("ERROR: Could not get usage value.");
						print_line("The specified report type is not valid.");
						break;
					case HIDP_STATUS_INCOMPATIBLE_REPORT_ID:
						print_line("ERROR: Could not get usage value.");
						print_line("The collection contains buttons on the specified usage page in a report of the specified type, but there are no such usages in the specified report.");
						break;
					case HIDP_STATUS_INVALID_PREPARSED_DATA:
						print_line("ERROR: Could not get usage value.");
						print_line("The preparsed data is not valid.");
						break;
					case HIDP_STATUS_USAGE_NOT_FOUND:
						print_line("ERROR: Could not get usage value.");
						print_line("The collection does not contain any buttons on the specified usage page in any report of the specified report type.");
						break;
					default:
						print_line("Successfully obtained usage value.");
						break;
					}

					bool usageValueSuccess = false;
					if (usageValueResult == HIDP_STATUS_SUCCESS) {
						print_line(vformat("Usage: %d, Usage value: %d", (unsigned int)valueCaps[i].Range.UsageMin, (unsigned int)value));
						usageValueSuccess = true;
					}


					if (false) {
						print_line("Getting usage value array");
						USHORT usageValueByteLength= valueCaps->BitSize * valueCaps->ReportCount;
						usageValueByteLength = (int)(valueCaps->BitSize);
						//print_line(vformat("Bit size: %d", (unsigned int)usageValueByteLength));
						//print_line(vformat("Usage value byte length: %d", (unsigned int)usageValueByteLength));

						LPBYTE usageValueArrayBuffer = new BYTE[(int)(usageValueByteLength)]{0};
						PCHAR usageValueArray = (PCHAR)usageValueArrayBuffer;

						unsigned long usageValueArrayResult = HidP_GetUsageValueArray(
							HidP_Input,
							deviceInfo.hid.usUsagePage,
							0,
							valueCaps[i].Range.UsageMin,
							usageValueArray,
							usageValueByteLength,
							devicePreparsedData,
							(PCHAR)raw->data.hid.bRawData,
							raw->data.hid.dwSizeHid * raw->data.hid.dwCount
						);

						switch (usageValueArrayResult)
						{
						case HIDP_STATUS_INVALID_REPORT_LENGTH:
							print_line("ERROR: Could not get usage value array.");
							print_line("The report length is not valid.");
							break;
						case HIDP_STATUS_INVALID_REPORT_TYPE:
							print_line("ERROR: Could not get usage value array.");
							print_line("The specified report type is not valid.");
							break;
						case HIDP_STATUS_NOT_VALUE_ARRAY:
							print_line("ERROR: Could not get usage value array.");
							print_line("The requested usage is not a usage value array.");
							break;
						case HIDP_STATUS_BUFFER_TOO_SMALL:
							print_line("ERROR: Could not get usage value array.");
							print_line("The UsageValue buffer is too small to hold the requested usage.");
							break;
						case HIDP_STATUS_INCOMPATIBLE_REPORT_ID:
							print_line("ERROR: Could not get usage value array.");
							print_line("The collection contains buttons on the specified usage page in a report of the specified type, but there are no such usages in the specified report.");
							break;
						case HIDP_STATUS_INVALID_PREPARSED_DATA:
							print_line("ERROR: Could not get usage value array.");
							print_line("The preparsed data is not valid.");
							break;
						case HIDP_STATUS_USAGE_NOT_FOUND:
							print_line("ERROR: Could not get usage value array.");
							print_line("The collection does not contain any buttons on the specified usage page in any report of the specified report type.");
							break;
						default:
							print_line("Successfully obtained usage value array.");
							break;
						}



						delete[] usageValueArrayBuffer;
					}

				} */

				/* LPBYTE reportBuffer = new BYTE[caps.InputReportByteLength]{};
				//reportBuffer[0] = (BYTE)HID_USAGE_DIGITIZER_TOUCH_PAD;
				PVOID inputReportData = (PVOID)reportBuffer;

				if (HidD_GetInputReport(hDevice, inputReportData, caps.InputReportByteLength) == false)
				{
					print_line(vformat("HidD_GetInputReport() failed. Error code: %d", (int)GetLastError()));
				}

				delete[] reportBuffer; */


				delete[] reportBuffer;
				delete[] valueCapsBuffer;
				delete[] buttonCapsBuffer;
				delete[] preparsedDataBuffer;
			}

			delete[] rawReportBuffer;
			break;
		/* else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}	 */
		}
	}
	WNDPROC origWndProc = DeviceManager::get_singleton()->getOrigWndProc();
	//DefWindowProcW(hWnd, uMsg, wParam, lParam);
	return CallWindowProcW(origWndProc, hWnd, uMsg, wParam, lParam);
}