extends Area2D

@export var speed: float = 320.0
@export var damage: int = 1
@export var lifetime: float = 3.0

var direction: Vector2 = Vector2.RIGHT
var age: float = 0.0
var spent: bool = false


func _physics_process(delta: float) -> void:
	position += direction * speed * delta

	# Enemies are Area2D, not physics bodies, so we look at overlapping
	# AREAS here -- get_overlapping_bodies() would never see the flower.
	if not spent:
		for area in get_overlapping_areas():
			if area.has_method("take_damage"):
				spent = true
				area.take_damage(damage)
				queue_free()
				return

	age += delta
	if age > lifetime:
		queue_free()
