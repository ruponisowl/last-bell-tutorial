extends Area2D

# The carnivorous flower.
# Sits still, watches for the player, spits seeds straight sideways
# so she can jump over them.

@export var detect_range: float = 220.0    # how far it can see, in pixels
@export var fire_cooldown: float = 1.5     # seconds between shots
@export var max_health: int = 3

@export var bullet_speed: float = 90.0     # lower = easier to dodge
@export var muzzle_height: float = -6.0    # how high the seed comes out
@export var muzzle_reach: float = 18.0     # how far in front it spawns

# CHANGE THIS PATH if you saved the bullet somewhere else.
const BULLET := preload("res://scene/flower_bullet.tscn")

@onready var anim: AnimatedSprite2D = $AnimatedSprite2D

var health: int
var cooldown: float = 0.0
var dead: bool = false
var player: Node2D = null


func _ready() -> void:
	health = max_health

	player = get_tree().get_first_node_in_group("player")
	if player == null:
		print("FLOWER: no node in group 'player'. ",
			  "Select your player -> Node tab -> Groups -> add 'player'.")

	anim.play("idle")


func _physics_process(delta: float) -> void:
	if dead or player == null:
		return

	cooldown -= delta

	var dist: float = global_position.distance_to(player.global_position)

	if dist <= detect_range:
		anim.flip_h = player.global_position.x < global_position.x

		if anim.animation != "attack":
			anim.play("attack")

		if cooldown <= 0.0:
			shoot()
			cooldown = fire_cooldown
	else:
		if anim.animation != "idle":
			anim.play("idle")


func shoot() -> void:
	var facing: float = -1.0
	if player.global_position.x > global_position.x:
		facing = 1.0

	var dir := Vector2(facing, 0.0)

	var b := BULLET.instantiate()
	b.position = global_position + Vector2(facing * muzzle_reach, muzzle_height)
	b.direction = dir
	b.speed = bullet_speed
	get_parent().add_child(b)

	
# ---------------------------------------------------------------------
#  TAKING DAMAGE
#  The paper plane calls this. It doesn't need to know anything else
#  about the flower -- same contract as take_damage() on the player.
# ---------------------------------------------------------------------
func take_damage(amount: int) -> void:
	if dead:
		return

	health -= amount
	print("FLOWER hit! health = ", health)
	var sfx = get_node_or_null("Hit")
	if sfx:
		sfx.play()
	if health <= 0:
		dead = true
		_die()
	else:
		_flash()


func _flash() -> void:
	# A modulate tint sits on top of whatever animation is playing, so
	# this shows even while it's mid-attack. Playing the "hurt" clip
	# wouldn't -- _physics_process would switch straight back to
	# "attack" on the next frame.
	modulate = Color(1, 0.4, 0.4)
	await get_tree().create_timer(0.15).timeout
	modulate = Color(1, 1, 1)


func _die() -> void:
	print("FLOWER died")
	set_physics_process(false)

	if anim.sprite_frames.has_animation("death"):
		anim.play("death")
		await anim.animation_finished

	queue_free()
