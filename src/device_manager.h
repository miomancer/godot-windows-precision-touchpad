#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"

extern "C"
{
	#include <windows.h>
    #include <hidsdi.h>
}	

#pragma comment(lib, "User32.lib")

using namespace godot;

class DeviceManager : public godot::RefCounted {
	GDCLASS(DeviceManager, godot::RefCounted)
	static DeviceManager *singleton;

protected:
	static void _bind_methods();
	HWND windowHandle;
	WNDPROC origWndProc;
	godot::Array touch_positions;

public:
	DeviceManager();
	~DeviceManager() override;

	static DeviceManager *get_singleton();

	godot::Array get_device_list();
	int register_touchpads();
	int set_window(int64_t window_handle);

	WNDPROC getOrigWndProc();
	godot::Vector2 get_touch_position(int index);
	void set_touch_position(int index, int x, int y);
};
