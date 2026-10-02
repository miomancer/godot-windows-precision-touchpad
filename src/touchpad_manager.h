#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "godot_cpp/classes/display_server.hpp"

extern "C"
{
	#include <windows.h>
    #include <hidsdi.h>
}	

#pragma comment(lib, "User32.lib")

using namespace godot;

class TouchpadManager : public godot::Node {
	GDCLASS(TouchpadManager, godot::Node)
	static TouchpadManager *singleton;

protected:
	static void _bind_methods();
	HWND windowHandle;
	WNDPROC origWndProc;
	godot::Array touch_positions;

	void _notification(int p_what);
public:
	TouchpadManager();
	~TouchpadManager() override;

	static TouchpadManager *get_singleton();

	godot::Array get_device_list();
	int register_touchpads();
	int replace_window_procedure();

	WNDPROC getOrigWndProc();
	godot::Vector2 get_touch_position(int index);
	void set_touch_position(int index, double x, double y);
	void update_touchpad_inputs();
};
