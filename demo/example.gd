extends Node


func _ready() -> void:
	var example := DeviceManager.new()
	print(example.get_device_list())
