extends HBoxContainer

@export var full_heart: Texture2D
@export var empty_heart: Texture2D
@export var heart_size: int = 32

var player: Node = null
var hearts: Array[TextureRect] = []


func _ready() -> void:
	add_theme_constant_override("separation", 6)
	_find_player()


func _find_player() -> void:
	player = get_tree().get_first_node_in_group("player")
	if player != null:
		_build(player.max_health)


# Builds one heart per point of max health, so changing max_health in
# the Inspector changes the bar with no code edits.
func _build(count: int) -> void:
	for h in hearts:
		h.queue_free()
	hearts.clear()

	for i in count:
		var t := TextureRect.new()
		t.texture = full_heart
		t.custom_minimum_size = Vector2(heart_size, heart_size)
		t.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
		add_child(t)
		hearts.append(t)


func _process(_delta: float) -> void:
	if player == null:
		_find_player()
		return

	for i in hearts.size():
		hearts[i].texture = full_heart if i < player.health else empty_heart
