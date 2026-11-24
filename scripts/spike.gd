extends Area2D

@export var damage: int = 1
@export var repeat_delay: float = 1.0

var _cooldown: float = 0.0

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _process(delta: float) -> void:
	if _cooldown > 0.0:
		_cooldown -= delta
		return
	# still standing in the spikes? hurt her again
	for b in get_overlapping_bodies():
		if b.has_method("take_damage"):
			b.take_damage(damage)
			_cooldown = repeat_delay
			break

func _on_body_entered(body: Node) -> void:
	if _cooldown > 0.0:
		return
	if body.has_method("take_damage"):
		body.take_damage(damage)
		_cooldown = repeat_delay
