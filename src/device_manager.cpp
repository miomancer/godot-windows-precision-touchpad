#include "device_manager.h"

void DeviceManager::_bind_methods() {
	godot::ClassDB::bind_method(godot::D_METHOD("get_device_list"), &DeviceManager::get_device_list);
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

std::string getDeviceName(PRAWINPUTDEVICELIST pRawInputDeviceList, int index) {
	HANDLE hDevice = pRawInputDeviceList[index].hDevice;
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
		std::string deviceName = getDeviceName(pRawInputDeviceList, i);
		print_line("Current device name: " + godot::String(deviceName.c_str()));
	}
	delete[] pRawInputDeviceList;
	return deviceList;
}

