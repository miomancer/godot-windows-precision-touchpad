extends Node

var device_manager : DeviceManager


func _ready() -> void:
	#Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	device_manager = DeviceManager.new()
	setup_tp()

func _process(delta):
	if Input.is_action_just_pressed("ui_accept"):
		print("ACCEPT")
	if Input.is_action_just_pressed("ui_cancel"):
		print("CANCEL")

func setup_tp():
	var window_id = 0 # Default ID for the main window
	var window_handle = DisplayServer.window_get_native_handle(DisplayServer.WINDOW_HANDLE, window_id)
	device_manager.set_window(window_handle)
	#device_manager.register_touchpads()
