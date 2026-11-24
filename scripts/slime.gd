extends Area2D

@export var speed: float = 40.0
@export var patrol_distance: float = 80.0
@export var start_direction: float = -1.0
@export var max_health: int = 2

@onready var anim: AnimatedSprite2D = $AnimatedSprite2D

var origin_x: float = 0.0
var direction: float = -1.0
var health: int = 0
var dead: bool = false


func _ready() -> void:
	origin_x = global_position.x
	direction = start_direction
	health = max_health
	anim.play("idle")
	body_entered.connect(_on_body_entered)


func _physics_process(delta: float) -> void:
	if dead:
		return

	global_position += Vector2(direction * speed * delta, 0.0)

	# Turn around at the ends, snapping back inside so it can never
	# get stuck oscillating on the boundary.
	var travelled := global_position.x - origin_x
	if abs(travelled) >= patrol_distance:
		direction = -direction
		global_position.x = origin_x + sign(travelled) * patrol_distance

	anim.flip_h = direction < 0.0


# One hit per touch. She has to leave and come back to be hurt again.
func _on_body_entered(body: Node2D) -> void:
	if dead:
		return
	if body.has_method("take_damage"):
		body.take_damage(1)


# Paper planes call this.
func take_damage(amount: int) -> void:
	if dead:
		return

	health -= amount
	print("SLIME hit! health = ", health)
	var sfx = get_node_or_null("Hit")
	if sfx:
		sfx.play()
	else:
		print("!! no Hit node on the slime")
	if health <= 0:
		dead = true
		_die()
	else:
		_flash()


func _flash() -> void:
	if anim.sprite_frames.has_animation("hurt"):
		anim.play("hurt")
		await anim.animation_finished
		if not dead:
			anim.play("idle")


func _die() -> void:
	print("SLIME died")
	set_physics_process(false)

	if anim.sprite_frames.has_animation("hurt"):
		anim.play("hurt")
		await anim.animation_finished

	queue_free()
