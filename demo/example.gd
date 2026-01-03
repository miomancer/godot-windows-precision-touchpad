extends Node


func _ready() -> void:
	var device_manager := DeviceManager.new()
	print(device_manager.get_device_list())
