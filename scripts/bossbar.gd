extends CanvasLayer

const BAR_W: float = 300.0
const BAR_H: float = 12.0
const EASE_SPEED: float = 1.4

var _root: Control
var _back: ColorRect
var _fill: ColorRect
var _name: Label
var _shown: float = 1.0
var _target: float = 1.0

func _ready() -> void:
	layer = 20
	_build()
	visible = false

func _build() -> void:
	_root = Control.new()
	_root.set_anchors_preset(Control.PRESET_FULL_RECT)
	_root.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(_root)

	_name = Label.new()
	_name.text = "GATE GUARD"
	_name.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	_name.add_theme_color_override("font_color", Color(0.95, 0.90, 0.80))
	_name.add_theme_font_size_override("font_size", 10)
	_root.add_child(_name)

	_back = ColorRect.new()
	_back.color = Color(0.10, 0.07, 0.09)
	_root.add_child(_back)

	_fill = ColorRect.new()
	_fill.color = Color(0.80, 0.18, 0.18)
	_root.add_child(_fill)

	_layout()
	get_viewport().size_changed.connect(_layout)

func _layout() -> void:
	var vp: Vector2 = get_viewport().get_visible_rect().size
	var x: float = (vp.x - BAR_W) * 0.5
	var y: float = vp.y - 42.0
	_name.position = Vector2(x, y - 18.0)
	_name.size = Vector2(BAR_W, 14.0)
	_back.position = Vector2(x - 2.0, y - 2.0)
	_back.size = Vector2(BAR_W + 4.0, BAR_H + 4.0)
	_fill.position = Vector2(x, y)
	_fill.size = Vector2(BAR_W, BAR_H)

func set_boss_health(current: int, maximum: int) -> void:
	if maximum <= 0:
		return
	_target = clampf(float(current) / float(maximum), 0.0, 1.0)
	visible = true

func hide_bar() -> void:
	visible = false

func _process(delta: float) -> void:
	if not visible:
		return
	_shown = move_toward(_shown, _target, delta * EASE_SPEED)
	_fill.size.x = BAR_W * _shown
