extends Area2D

var done: bool = false


func _ready() -> void:
	body_entered.connect(_on_body_entered)


func _on_body_entered(body: Node2D) -> void:
	if done:
		return
	if not body.is_in_group("player"):
		return

	done = true

	var mood = get_tree().get_current_scene().find_child("Mood", true, false)
	if mood:
		mood.set_sprint(true)
