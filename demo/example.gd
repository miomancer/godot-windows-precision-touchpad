extends Node


func _ready() -> void:
	var device_manager := DeviceManager.new()
	#print(device_manager.get_device_list())
	var window_id = 0 # Default ID for the main window
	var window_handle = DisplayServer.window_get_native_handle(DisplayServer.WINDOW_HANDLE, window_id)
	print("Window Handle: ", window_handle)
	device_manager.set_window(window_handle)
	print(device_manager.register_touchpads())
	
