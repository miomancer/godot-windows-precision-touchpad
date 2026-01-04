#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"

#pragma comment(lib, "User32.lib")

using namespace godot;

class DeviceManager : public godot::RefCounted {
	GDCLASS(DeviceManager, godot::RefCounted)

protected:
	static void _bind_methods();
	int windowHandle = -1;

public:
	DeviceManager() = default;
	~DeviceManager() override = default;

	godot::Array get_device_list();
	int register_touchpads();
	int set_window(int window_handle);
};
