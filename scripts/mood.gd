extends CanvasModulate

const NORMAL := Color(1, 1, 1)
const BOSS   := Color(0.62, 0.55, 0.80)   # cold violet
const SHAFT  := Color(0.34, 0.36, 0.52)   # dark and gloomy
const SPRINT := Color(1.00, 0.97, 0.88)   # warm daylight
var locked: bool = false
@export var fade_time: float = 0.8

@onready var boss_player: AudioStreamPlayer   = $BossMusic
@onready var shaft_player: AudioStreamPlayer  = $ShaftMusic
@onready var sprint_player: AudioStreamPlayer = $SprintMusic
@onready var parking_player: AudioStreamPlayer = $ParkingMusic

func _ready() -> void:
	color = NORMAL


func set_parking(on: bool) -> void:
	if locked: return
	_set_mood(NORMAL, parking_player, on)

func set_boss(on: bool) -> void:
	_set_mood(BOSS, boss_player, on)


func set_shaft(on: bool) -> void:
	if locked: return
	if on:
		$ShaftEnter.play()
	_set_mood(SHAFT, shaft_player, on)


func set_sprint(on: bool) -> void:
	locked = on
	_set_mood(SPRINT, sprint_player, on)


# Every mood works the same: tint the world, swap the music. Stopping
# all three first means moving straight from one mood to another can
# never leave two tracks playing at once.
func _set_mood(tint: Color, player: AudioStreamPlayer, on: bool) -> void:
	var t := create_tween()
	t.tween_property(self, "color", tint if on else NORMAL, fade_time)

	for p in [boss_player, shaft_player, sprint_player, parking_player]:
		if p: p.stop()

	var bg := _find_background()

	if on:
		if bg:
			bg.stream_paused = true
		if player:
			player.play()
	else:
		if bg:
			if not bg.playing:
				bg.play()
			bg.stream_paused = false


func _find_background() -> Node:
	var n = get_node_or_null("/root/Bmusic")
	if n:
		if n is AudioStreamPlayer:
			return n
		for c in n.get_children():
			if c is AudioStreamPlayer:
				return c
	return get_tree().get_current_scene().find_child("Bmusic", true, false)
func reset_all() -> void:
	locked = false
	var t := create_tween()
	t.tween_property(self, "color", NORMAL, 0.3)
	for p in [boss_player, shaft_player, sprint_player, parking_player]:
		if p:
			p.stop()
	var bg := _find_background()
	if bg:
		if not bg.playing:
			bg.play()
		bg.stream_paused = false
	
