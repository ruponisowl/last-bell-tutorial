extends CanvasLayer

# --- look -------------------------------------------------------------
const DIM      := Color(0.04, 0.02, 0.07, 0.80)   # the darkened screen
const GOLD     := Color(1.0, 0.82, 0.22)          # matches your CG counter
const INK      := Color(0.10, 0.07, 0.02)         # dark text on gold

# Optional: use the same pixel font as the CG counter.
# Right-click your font in FileSystem -> Copy Path, paste between the quotes.
const FONT_PATH := ""
var selected: int = 0
var _buttons: Array = []
var is_pause: bool = false
func _input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_cancel"):
		if visible and is_pause:
			_resume()
		elif not visible:
			is_pause = true
			show_game_over("PAUSED", "")
		return

	if not visible:
		return

	if event.is_action_pressed("ui_down"):
		selected = (selected + 1) % _buttons.size()
		_refresh_menu()
		get_viewport().set_input_as_handled()
	elif event.is_action_pressed("ui_up"):
		selected = (selected - 1 + _buttons.size()) % _buttons.size()
		_refresh_menu()
		get_viewport().set_input_as_handled()
	elif event.is_action_pressed("ui_accept"):
		if selected == 0:
			_on_restart()
		else:
			_on_quit()
		get_viewport().set_input_as_handled()

func _resume() -> void:
	is_pause = false
	visible = false
	get_tree().paused = false
func _ready() -> void:
	visible = false
	process_mode = Node.PROCESS_MODE_ALWAYS

	_style()
	

	$Panel/VBox/RestartButton.pressed.connect(_on_restart)
	$Panel/VBox/QuitButton.pressed.connect(_on_quit)

	var sub: Label = $Panel/VBox/SubLabel
	sub.size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	sub.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	sub.add_theme_font_size_override("font_size", 22)
	sub.add_theme_color_override("font_color", Color(0.85, 0.85, 0.92))
func show_game_over(title: String = "GAME OVER", subtitle: String = "") -> void:
	$Panel/VBox/TitleLabel.text = title

	var sub: Label = $Panel/VBox/SubLabel
	sub.text = subtitle
	sub.visible = subtitle != ""

	visible = true
	get_tree().paused = true
	selected = 0
	_refresh_menu()

# ----------------------------------------------------------------------
#  All the appearance lives here so the logic above stays readable.
# ----------------------------------------------------------------------
func _style() -> void:
	var panel: ColorRect = $Panel
	panel.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	panel.color = DIM

	# Fill the whole screen, then centre the children inside it. This way
	# the position never depends on how big the buttons turn out to be.
	var box: VBoxContainer = $Panel/VBox
	box.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	box.alignment = BoxContainer.ALIGNMENT_CENTER
	box.add_theme_constant_override("separation", 24)

	var title: Label = $Panel/VBox/TitleLabel
	title.size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	title.add_theme_font_size_override("font_size", 56)
	title.add_theme_color_override("font_color", GOLD)
	title.add_theme_color_override("font_shadow_color", Color(0, 0, 0, 0.85))
	title.add_theme_constant_override("shadow_offset_x", 4)
	title.add_theme_constant_override("shadow_offset_y", 4)

	_style_button($Panel/VBox/RestartButton)
	_style_button($Panel/VBox/QuitButton)

	if FONT_PATH != "":
		var f: Font = load(FONT_PATH)
		if f:
			title.add_theme_font_override("font", f)
			$Panel/VBox/RestartButton.add_theme_font_override("font", f)
			$Panel/VBox/QuitButton.add_theme_font_override("font", f)


func _style_button(b: Button) -> void:
	b.size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	b.focus_mode = Control.FOCUS_NONE
	b.custom_minimum_size = Vector2(280, 60)
	b.add_theme_font_size_override("font_size", 28)
	b.add_theme_color_override("font_color", Color.WHITE)
	b.add_theme_color_override("font_hover_color", INK)
	b.add_theme_color_override("font_pressed_color", INK)

	b.add_theme_stylebox_override("normal",  _box(Color(0.10, 0.09, 0.13, 0.95), GOLD))
	b.add_theme_stylebox_override("hover",   _box(GOLD, GOLD))
	b.add_theme_stylebox_override("pressed", _box(Color(0.78, 0.62, 0.14), GOLD))
	b.add_theme_stylebox_override("focus",   _box(Color(0, 0, 0, 0), GOLD))


func _box(fill: Color, border: Color) -> StyleBoxFlat:
	var s := StyleBoxFlat.new()
	s.bg_color = fill
	s.border_color = border
	s.set_border_width_all(3)
	s.set_corner_radius_all(8)
	s.content_margin_left = 20
	s.content_margin_right = 20
	s.content_margin_top = 12
	s.content_margin_bottom = 12
	return s


func _on_restart() -> void:
	get_tree().paused = false
	get_tree().reload_current_scene()


func _on_quit() -> void:
	get_tree().quit()
func _refresh_menu() -> void:
	if _buttons.is_empty():
		_buttons = [$Panel/VBox/RestartButton, $Panel/VBox/QuitButton]
	for i in _buttons.size():
		var b: Button = _buttons[i]
		if i == selected:
			b.add_theme_stylebox_override("normal", _box(GOLD, GOLD))
			b.add_theme_color_override("font_color", INK)
		else:
			b.add_theme_stylebox_override("normal", _box(INK, GOLD))
			b.add_theme_color_override("font_color", Color.WHITE)
