/* Last Bell: Campus Rush -- a readable sign
 *
 * She walks into it, it says its piece one line at a time, and then
 * it is done for the rest of the run. Uses the same hand-written
 * StringQueue as the guard's dialogue.
 */

#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/variant/variant.hpp>

#include "D:/last-bell/scripts/string_queue.h"

using namespace godot;
using namespace jenova::sdk;

JENOVA_SCRIPT_BEGIN

const int SIGN_WAITING = 0;
const int SIGN_READING = 1;
const int SIGN_DONE    = 2;

StringQueue signLines;
int   signState = SIGN_WAITING;
Node* lastScene = nullptr;


void OnReady(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);
	Node* scene = self->get_tree()->get_current_scene();

	if (scene != lastScene)
	{
		lastScene = scene;
		signLines.clear();
		signState = SIGN_WAITING;
		Output("[SIGN] waiting to be read");
	}
}


void OnProcess(Caller* instance, double delta)
{
	if (signState == SIGN_DONE) return;

	Area2D* self = GetSelf<Area2D>(instance);
	Node* scene = self->get_tree()->get_current_scene();
	Node* box = scene->find_child("DialogueBox", true, false);

	// --------------------------------------------------------------
	//  She has not reached it yet
	// --------------------------------------------------------------
	if (signState == SIGN_WAITING)
	{
		if (!self->has_overlapping_bodies()) return;

		// Change these lines to change what the sign says.
		signLines.clear();
		signLines.enqueue("NOTICE:  LIFT OUT OF ORDER");
		signLines.enqueue("NOTICE:  Engineer has been informed. (12/03)");
		signLines.enqueue("YOU:     ...of course it is.");
		signLines.enqueue("No stairs on this side.");
signLines.enqueue("Service shaft's open. I'm climbing.");

		signState = SIGN_READING;
		Output("[SIGN] being read");

		if (box != nullptr && signLines.peek() != nullptr)
			box->call("show_line", String(signLines.peek()));

		return;
	}

	// --------------------------------------------------------------
	//  Reading: one line per press of E
	// --------------------------------------------------------------
	Input* input = Input::get_singleton();
	if (input == nullptr) return;
	if (!input->is_action_just_pressed("talk")) return;

	char spoken[MAX_LINE];
	signLines.dequeue(spoken);

	if (signLines.isEmpty())
	{
		signState = SIGN_DONE;
		if (box != nullptr) box->call("hide_box");
		Output("[SIGN] read -- queue empty");
		return;
	}

	if (box != nullptr)
		box->call("show_line", String(signLines.peek()));
}

JENOVA_SCRIPT_END
