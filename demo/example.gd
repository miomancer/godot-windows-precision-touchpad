extends Node

var  : TouchpadManager


func _ready() -> void:
	#Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	 = TouchpadManager.new()
	#Engine.register_singleton()
	setup_tp()

func _process(delta):
	pass
	if Input.is_action_just_pressed("ui_accept"):
		print("ACCEPT")
	if Input.is_action_just_pressed("ui_cancel"):
		print("CANCEL")
	print([.get_touch_position(0),
	.get_touch_position(1),
	.get_touch_position(2),
	.get_touch_position(3),
	.get_touch_position(4)])

func setup_tp():
	var window_id = 0 # Default ID for the main window
	var window_handle = DisplayServer.window_get_native_handle(DisplayServer.WINDOW_HANDLE, window_id)
	.replace_window_procedure(window_handle)
	.register_touchpads()
