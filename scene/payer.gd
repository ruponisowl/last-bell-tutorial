extends CharacterBody2D

@export var speed: float = 130.0
@export var jump_velocity: float = -280.0
@export var max_health: int = 3

@onready var anim: AnimatedSprite2D = $AnimatedSprite2D
# --- respawn tracking ---
var safe_last: Vector2 = Vector2.ZERO
var safe_prev: Vector2 = Vector2.ZERO
var _safe_timer: float = 0.0
var health: int
var invincible: bool = false
var level_start: Vector2 = Vector2.ZERO
const PAPER_PLANE := preload("res://scene/paper_plane.tscn")

@export var throw_cooldown: float = 0.35
var throw_timer: float = 0.0

func _ready() -> void:
	level_start = global_position
	health = max_health
	print("Player health: ", health)
	safe_last = global_position
	safe_prev = global_position
	$Gun.visible = false
func _physics_process(delta: float) -> void:
	# --- gravity ------------------------------------------------------
	if not is_on_floor():
		velocity += get_gravity() * delta

	# --- jump ---------------------------------------------------------
	if Input.is_action_just_pressed("jump") and is_on_floor():
		velocity.y = jump_velocity

	# --- left and right -----------------------------------------------
	var direction := Input.get_axis("left", "right")

	if direction != 0.0:
		velocity.x = direction * speed
		anim.flip_h = direction < 0.0
		$Gun.flip_h = direction < 0.0
		$Gun.position.x = -7.0 if direction < 0.0 else 7.0
		play_once("run")
	else:
		velocity.x = move_toward(velocity.x, 0.0, speed)
		play_once("idle")
	throw_timer -= delta
	$Gun.visible = Input.is_action_pressed("throw")
	if Input.is_action_just_pressed("throw") and throw_timer <= 0.0:
		_throw()
	move_and_slide()
	_safe_timer -= delta
	if is_on_floor() and _safe_timer <= 0.0:
		_safe_timer = 0.5
		safe_prev = safe_last
		safe_last = global_position


# Only call play() when the animation actually needs to change,
# otherwise it restarts from frame 0 sixty times a second.
func play_once(name: String) -> void:
	if anim.animation != name:
		anim.play(name)


# ---------------------------------------------------------------------
#  TAKING DAMAGE
#  The flower's bullet calls this. Anything that can hurt her just
#  needs to call take_damage() -- it doesn't need to know anything
#  else about her.
# ---------------------------------------------------------------------
func take_damage(amount: int) -> void:
	if invincible:
		return

	health -= amount
	print("Ouch! health = ", health)

	if has_node("Hurt"):
		$Hurt.play()
	if health <= 0:
		restart_level()
		return
	# Returns false if she is already on full hearts, so the fruit knows
# not to vanish -- she can come back for it later.
func heal(amount: int) -> bool:
	if health >= max_health:
		return false

	health = min(health + amount, max_health)
	print("Healed! health = ", health)

	var sfx = get_node_or_null("Heal")
	if sfx:
		sfx.play()

	return true




func die() -> void:
	print("You died!")

	var panel = get_tree().get_first_node_in_group("gameover")
	if panel:
		panel.show_game_over("GAME OVER")
	else:
		get_tree().reload_current_scene()
func _throw() -> void:
	throw_timer = throw_cooldown
	if has_node("Shot"):
		$Shot.play()
	var facing := -1.0 if anim.flip_h else 1.0

	var p := PAPER_PLANE.instantiate()
	get_parent().add_child(p)

	# global_position AFTER add_child -- world space, so a parent
	# transform can't offset it. Safe because the plane's mask
	# ignores the ground entirely.
	p.global_position = global_position + Vector2(facing * 14.0, -12.0)
	p.direction = Vector2(facing, 0.0)

	if p.has_node("Sprite2D"):
		p.get_node("Sprite2D").flip_h = facing < 0.0
	var flash := $Gun/Flash
	flash.visible = true
	await get_tree().create_timer(0.05).timeout
	if is_instance_valid(flash):
		flash.visible = false
func respawn() -> void:
	var target: Vector2 = safe_prev
	if target == Vector2.ZERO:
		target = safe_last
	velocity = Vector2.ZERO
	global_position = target
	var cam := get_node_or_null("Camera2D")
	if cam:
		cam.reset_smoothing()
	take_damage(1)
func set_level_start(pos: Vector2) -> void:
	level_start = pos

func restart_level() -> void:
	velocity = Vector2.ZERO
	global_position = level_start
	safe_last = level_start
	safe_prev = level_start
	var cam := get_node_or_null("Camera2D")
	var mood = get_tree().get_current_scene().find_child("Mood", true, false)
	if mood and mood.has_method("reset_all"):
		mood.reset_all()
	if cam:
		cam.reset_smoothing()
	# refill hearts using your existing heal path, so the HP bar updates too
	for i in range(20):
		if not heal(1):
			break
