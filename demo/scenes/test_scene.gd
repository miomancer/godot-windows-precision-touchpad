extends Node2D

var device_manager : DeviceManager
@onready var sprites = $Sprites


func _ready() -> void:
	#Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	device_manager = DeviceManager.new()
	#Engine.register_singleton(device_manager)
	setup_tp()

func _process(delta):
	print([device_manager.get_touch_position(0),
	device_manager.get_touch_position(1),
	device_manager.get_touch_position(2),
	device_manager.get_touch_position(3),
	device_manager.get_touch_position(4)])
	
	for i in range(sprites.get_child_count()):
		var sprite = sprites.get_child(i)
		if device_manager.get_touch_position(i).x > 0:
			sprite.show()
			sprite.position = device_manager.get_touch_position(i) / Vector2(1404.0, 864.0) * get_viewport_rect().size

func setup_tp():
	var window_id = 0 # Default ID for the main window
	var window_handle = DisplayServer.window_get_native_handle(DisplayServer.WINDOW_HANDLE, window_id)
	device_manager.set_window(window_handle)
	device_manager.register_touchpads()	
