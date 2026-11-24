/* Last Bell: Campus Rush -- the 8:50 clock */

#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/label.hpp>
#include <Godot/classes/audio_stream_player.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/variant/variant.hpp>
#include <Godot/variant/color.hpp>
#include <cstdio>

using namespace godot;
using namespace jenova::sdk;

JENOVA_SCRIPT_BEGIN

// ------------------------------------------------------------------
//  Settings
// ------------------------------------------------------------------
const int START_HOUR   = 8;
const int START_MINUTE = 50;
const int END_HOUR     = 9;
const int END_MINUTE   = 0;

// Game seconds per real second. 1.0 = a true ten-minute run.
const double CLOCK_SPEED = 1.0;

const int    PANIC_SECONDS  = 60;    // clock turns red below this
const double BANNER_SECONDS = 4.0;   // how long the warning stays up
const double PANEL_DELAY    = 1.6;   // bell rings this long before the panel
const double FULL_TURN      = 6.283185307179586;

// ------------------------------------------------------------------
//  State. Jenova shares these across every node using this script,
//  so we reset them ourselves whenever the level reloads.
// ------------------------------------------------------------------
double elapsed    = 0.0;
bool   started    = false;   // false while she is still in the tutorial
bool   finished   = false;
bool   panelShown = false;
double sinceBell  = 0.0;
double bannerTime = 0.0;
Node*  lastScene  = nullptr;

void OnReady(Caller* instance)
{
	Node2D* self = GetSelf<Node2D>(instance);
	Node* scene = self->get_tree()->get_current_scene();

	if (scene != lastScene)
	{
		lastScene  = scene;
		elapsed    = 0.0;
		started    = false;
		finished   = false;
		panelShown = false;
		sinceBell  = 0.0;
		bannerTime = 0.0;
		Output("[CLOCK] waiting -- tutorial section");
	}

	// Hide the dial until she leaves the tutorial.
	self->set_visible(false);

	Label* label = Object::cast_to<Label>(scene->find_child("ClockLabel", true, false));
	if (label != nullptr) label->set_visible(false);

	Label* banner = Object::cast_to<Label>(scene->find_child("Banner", true, false));
	if (banner != nullptr) banner->set_visible(false);
}

void OnProcess(Caller* instance, double delta)
{
	Node2D* self = GetSelf<Node2D>(instance);
	Node* scene = self->get_tree()->get_current_scene();

	// --------------------------------------------------------------
	//  Still in the tutorial: watch the gate, run no clock.
	// --------------------------------------------------------------
	if (!started)
	{
		Area2D* gate = Object::cast_to<Area2D>(
			scene->find_child("ClockTrigger", true, false));

		if (gate != nullptr && gate->has_overlapping_bodies())
		{
			started = true;
			bannerTime = 0.0;
			self->set_visible(true);

			Label* label = Object::cast_to<Label>(scene->find_child("ClockLabel", true, false));
			if (label != nullptr) label->set_visible(true);

			Label* banner = Object::cast_to<Label>(scene->find_child("Banner", true, false));
			if (banner != nullptr)
			{
				banner->set_text(String("CLASS STARTS AT 9:00 -- RUN!"));
				banner->set_visible(true);
			}

			Output("[CLOCK] tutorial cleared -- clock running");
		}
		return;
	}

	// --------------------------------------------------------------
	//  Out of time: let the bell ring, then show the panel.
	// --------------------------------------------------------------
	if (finished)
	{
		if (!panelShown)
		{
			sinceBell += delta;
			if (sinceBell >= PANEL_DELAY)
			{
				panelShown = true;
				Node* panel = self->get_tree()->get_first_node_in_group("gameover");
				if (panel != nullptr)
					panel->call("show_game_over", String("YOU'RE LATE"));
			}
		}
		return;
	}

	// --------------------------------------------------------------
	//  Normal ticking
	// --------------------------------------------------------------
	if (bannerTime < BANNER_SECONDS)
	{
		bannerTime += delta;
		if (bannerTime >= BANNER_SECONDS)
		{
			Label* banner = Object::cast_to<Label>(scene->find_child("Banner", true, false));
			if (banner != nullptr) banner->set_visible(false);
		}
	}

	elapsed += delta * CLOCK_SPEED;

	int startSec = START_HOUR * 3600 + START_MINUTE * 60;
	int endSec   = END_HOUR   * 3600 + END_MINUTE   * 60;

	double nowExact = (double)startSec + elapsed;
	if (nowExact >= (double)endSec)
	{
		nowExact = (double)endSec;
		finished = true;
	}

	int nowSec = (int)nowExact;
	int h = nowSec / 3600;
	int m = (nowSec / 60) % 60;
	int s = nowSec % 60;
	double frac = nowExact - (double)nowSec;

	// Hands point up at rotation 0, and Godot's positive rotation runs
	// clockwise on screen, so a plain fraction of a turn is all we need.
	double secTurn  = ((double)s + frac) / 60.0 * FULL_TURN;
	double minTurn  = ((double)m + (double)s / 60.0) / 60.0 * FULL_TURN;
	double hourTurn = ((double)(h % 12) + (double)m / 60.0) / 12.0 * FULL_TURN;

	Node2D* hand;

	hand = Object::cast_to<Node2D>(self->get_node_or_null(NodePath("Hour")));
	if (hand != nullptr) hand->set_rotation(hourTurn);

	hand = Object::cast_to<Node2D>(self->get_node_or_null(NodePath("Minute")));
	if (hand != nullptr) hand->set_rotation(minTurn);

	hand = Object::cast_to<Node2D>(self->get_node_or_null(NodePath("Second")));
	if (hand != nullptr) hand->set_rotation(secTurn);

	Label* label = Object::cast_to<Label>(scene->find_child("ClockLabel", true, false));
	if (label != nullptr)
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%d:%02d AM", h, m);
		label->set_text(String(buf));

		if (endSec - nowSec <= PANIC_SECONDS)
			label->set_modulate(Color(1.0f, 0.35f, 0.35f));
		else
			label->set_modulate(Color(1.0f, 1.0f, 1.0f));
	}

	if (finished)
	{
		Output("[CLOCK] 9:00 -- out of time");

		AudioStreamPlayer* bell = Object::cast_to<AudioStreamPlayer>(
			self->get_node_or_null(NodePath("Bell")));
		if (bell != nullptr) bell->play();
	}
}

JENOVA_SCRIPT_END
