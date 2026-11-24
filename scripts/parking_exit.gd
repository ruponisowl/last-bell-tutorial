extends Area2D

@export var turn_on: bool = true

var done: bool = false

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _on_body_entered(body: Node) -> void:
	if done:
		return
	if not body.has_method("take_damage"):
		return
	done = true
	var mood = get_tree().get_current_scene().find_child("Mood", true, false)
	if mood:
		mood.set_parking(turn_on)
