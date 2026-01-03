#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"

#include <windows.h>

#pragma comment(lib, "User32.lib")

using namespace godot;

std::string wStringToString(const wchar_t* wstr, int size);
std::string getDeviceName(PRAWINPUTDEVICELIST pRawInputDeviceList, int index);

class DeviceManager : public godot::RefCounted {
	GDCLASS(DeviceManager, godot::RefCounted)

protected:
	static void _bind_methods();

public:
	DeviceManager() = default;
	~DeviceManager() override = default;

	godot::Array get_device_list();
};
