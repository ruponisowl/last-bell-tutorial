extends Area2D

@export var level_number: int = 2

func _ready() -> void:
	body_entered.connect(_on_body_entered)

func _on_body_entered(body: Node) -> void:
	if not body.has_method("set_level_start"):
		return

	# use the collision shape's real world position, not the node origin
	var spot: Vector2 = global_position
	var cs := get_node_or_null("CollisionShape2D")
	if cs:
		spot = cs.global_position
		if cs.shape is RectangleShape2D:
			spot.y += cs.shape.size.y * 0.5 - 8.0

	body.set_level_start(spot)
	print("[LEVEL] level ", level_number, " checkpoint at ", spot)
