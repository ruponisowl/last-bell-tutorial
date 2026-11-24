extends Area2D

@export var speed: float = 90.0
@export var damage: int = 1
@export var lifetime: float = 4.0

var direction: Vector2 = Vector2.RIGHT
var age: float = 0.0
var spent: bool = false     # a seed may only ever hit once


func _ready() -> void:
	$Sprite2D.texture = load("res://sprites/flower_bullet (1).png")


func _physics_process(delta: float) -> void:
	position += direction * speed * delta

	if not spent:
		for body in get_overlapping_bodies():
			if body.has_method("take_damage"):
				spent = true
				body.take_damage(damage)
				queue_free()
				return

	age += delta
	if age > lifetime:
		queue_free()
