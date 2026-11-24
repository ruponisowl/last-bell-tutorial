extends CanvasLayer

const GOLD := Color(1.0, 0.82, 0.22)
const FONT_PATH := ""     # e.g. "res://fonts/yourfont.ttf"


func _ready() -> void:
	_style()
	visible = false


# Called from guard.cpp
func show_line(text: String) -> void:
	$Panel/VBox/LineLabel.text = text
	visible = true


func hide_box() -> void:
	visible = false


func _style() -> void:
	var panel: Panel = $Panel
	panel.anchor_left   = 0.0
	panel.anchor_top    = 1.0
	panel.anchor_right  = 1.0
	panel.anchor_bottom = 1.0
	panel.offset_left   = 40
	panel.offset_right  = -40
	panel.offset_top    = -180
	panel.offset_bottom = -30

	var sb := StyleBoxFlat.new()
	sb.bg_color = Color(0.04, 0.02, 0.07, 0.93)
	sb.border_color = GOLD
	sb.set_border_width_all(3)
	sb.set_corner_radius_all(8)
	panel.add_theme_stylebox_override("panel", sb)

	var box: VBoxContainer = $Panel/VBox
	box.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	box.offset_left   = 24
	box.offset_top    = 18
	box.offset_right  = -24
	box.offset_bottom = -14
	box.add_theme_constant_override("separation", 10)

	var line: Label = $Panel/VBox/LineLabel
	line.add_theme_font_size_override("font_size", 26)
	line.add_theme_color_override("font_color", Color.WHITE)
	line.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	line.size_flags_vertical = Control.SIZE_EXPAND_FILL

	var hint: Label = $Panel/VBox/HintLabel
	hint.text = "[E] continue"
	hint.add_theme_font_size_override("font_size", 16)
	hint.add_theme_color_override("font_color", GOLD)
	hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	if FONT_PATH != "":
		var f: Font = load(FONT_PATH)
		if f:
			line.add_theme_font_override("font", f)
			hint.add_theme_font_override("font", f)
