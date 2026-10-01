extends Node2D

var  : TouchpadManager
@onready var sprites = $Sprites


func _ready() -> void:
	#get_tree().set_auto_accept_quit(false)
	#Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	 = TouchpadManager.new()
	#Engine.register_singleton()
	setup_tp()

func _process(delta):
	#print([.get_touch_position(0),
	#.get_touch_position(1),
	#.get_touch_position(2),
	#.get_touch_position(3),
	#.get_touch_position(4)])
	
	for i in range(sprites.get_child_count()):
		var sprite = sprites.get_child(i)
		if .get_touch_position(i).x > 0:
			sprite.show()
			sprite.position = .get_touch_position(i) * get_viewport_rect().size
		else:
			sprite.hide()


func setup_tp():
	var window_id = 0 # Default ID for the main window
	var window_handle = DisplayServer.window_get_native_handle(DisplayServer.WINDOW_HANDLE, window_id)
	.replace_window_procedure(window_handle)
	.register_touchpads()


#func _notification(what):
	#if what == NOTIFICATION_WM_CLOSE_REQUEST:
		# = null
		#get_tree().quit() # default behavior
