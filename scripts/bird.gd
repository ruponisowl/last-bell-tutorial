extends Area2D

@export var speed: float = 90.0
@export var patrol_distance: float = 140.0
@export var bob_height: float = 10.0
@export var bob_speed: float = 3.0
@export var start_direction: float = -1.0

@onready var anim: AnimatedSprite2D = $AnimatedSprite2D

var origin: Vector2
var direction: float = -1.0
var t: float = 0.0


func _ready() -> void:
	origin = global_position
	direction = start_direction
	anim.play("fly")
	body_entered.connect(_on_body_entered)


func _physics_process(delta: float) -> void:
	t += delta

	var x := global_position.x + direction * speed * delta
	var travelled := x - origin.x
	if abs(travelled) >= patrol_distance:
		direction = -direction
		x = origin.x + sign(travelled) * patrol_distance

	# Horizontal patrol plus a gentle rise and fall, so it never
	# travels in a straight line she can tune out.
	global_position = Vector2(x, origin.y + sin(t * bob_speed) * bob_height)

	anim.flip_h = direction < 0.0


func _on_body_entered(body: Node2D) -> void:
	if body.has_method("take_damage"):
		body.take_damage(1)
