/* Last Bell: Campus Rush -- the classroom door, and the ending */

#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/label.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/variant/variant.hpp>

using namespace godot;
using namespace jenova::sdk;

JENOVA_SCRIPT_BEGIN

bool  entered   = false;
bool  prompting = false;
Node* lastScene = nullptr;

void OnReady(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);
	Node* scene = self->get_tree()->get_current_scene();

	if (scene != lastScene)
	{
		lastScene = scene;
		entered   = false;
		prompting = false;
		Output("[DOOR] room 301 waiting");
	}
}

void OnProcess(Caller* instance, double delta)
{
	if (entered) return;

	Area2D* self = GetSelf<Area2D>(instance);
	Node* scene = self->get_tree()->get_current_scene();
	Node* box = scene->find_child("DialogueBox", true, false);

	bool near = self->has_overlapping_bodies();

	// Show and hide the prompt as she comes and goes. Reusing the
	// dialogue box means the prompt already matches the game's look.
	if (near && !prompting)
	{
		prompting = true;
		if (box != nullptr)
			box->call("show_line", String("ROOM 301  --  press E to go in"));
	}
	else if (!near && prompting)
	{
		prompting = false;
		if (box != nullptr) box->call("hide_box");
	}

	if (!near) return;

	Input* input = Input::get_singleton();
	if (input == nullptr) return;
	if (!input->is_action_just_pressed("talk")) return;

	// ------------------------------------------------------------
	//  She made it.
	// ------------------------------------------------------------
	entered = true;
	if (box != nullptr) box->call("hide_box");

	// Read the result straight off the HUD she has been watching.
	String timeText = String("");
	Label* clockLabel = Object::cast_to<Label>(
		scene->find_child("ClockLabel", true, false));
	if (clockLabel != nullptr) timeText = clockLabel->get_text();

	String cgText = String("");
	Label* cgLabel = Object::cast_to<Label>(
		scene->find_child("CGLabel", true, false));
	if (cgLabel != nullptr) cgText = cgLabel->get_text();

	String subtitle = String("Arrived ") + timeText
					+ String("      ") + cgText;

	Output("[DOOR] she made it to class");

	// The panel pauses the tree, which stops the clock for us.
	Node* panel = self->get_tree()->get_first_node_in_group("gameover");
	if (panel != nullptr)
		panel->call("show_game_over", String("YOU MADE IT!"), subtitle);
}

JENOVA_SCRIPT_END 
