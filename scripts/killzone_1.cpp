/* =====================================================================
   Killzone  --  RESPAWN VERSION

   She falls in -> she is sent back to her last safe ground and loses
   one heart. No scene reload, no slow motion.

   Requires respawn() to exist in payer.gd before you rebuild.
   ===================================================================== */

// Godot SDK
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/engine.hpp>
#include <Godot/classes/scene_tree.hpp>

// Jenova SDK
#include <JenovaSDK.h>

using namespace godot;
using namespace jenova::sdk;

JENOVA_CLASS_NAME("Killzone")

double respawnCooldown = 0.0;     // stops one fall firing twice
double reportTimer     = 0.0;     // for the once-a-second report

const double RESPAWN_LOCK = 0.6;

// Turn this on if you need to see what the killzone can see.
const bool DEBUG_REPORT = false;


// Script Block Begin
JENOVA_SCRIPT_BEGIN

// Find the player by name. Your node is called "payer", but we try a
// few spellings so this keeps working if you rename it.
Node2D* FindPlayer()
{
	Node* scene = GetTree()->get_current_scene();
	if (scene == nullptr) return nullptr;

	const char* names[] = { "payer", "player", "main player", "main_player" };
	for (int i = 0; i < 4; i++)
	{
		Node* n = scene->find_child(names[i], true, false);
		Node2D* p = Object::cast_to<Node2D>(n);
		if (p != nullptr) return p;
	}
	return nullptr;
}


void OnReady(Caller* instance)
{
	respawnCooldown = 0.0;
	reportTimer     = 0.0;

	// insurance, in case an older build left the game in slow motion
	Engine::get_singleton()->set_time_scale(1.0);

	Area2D* self = GetSelf<Area2D>(instance);
	if (self == nullptr)
	{
		Output("KILLZONE: GetSelf failed -- is this script on an Area2D?");
		return;
	}

	Output("KILLZONE ready.  my y = %.0f   monitoring = %s",
		   self->get_global_position().y,
		   self->is_monitoring() ? "ON" : "OFF");
}


void OnProcess(Caller* instance, double delta)
{
	Area2D* self = GetSelf<Area2D>(instance);
	if (self == nullptr) return;

	// ---- once-a-second report ----
	if (DEBUG_REPORT)
	{
		reportTimer += delta;
		if (reportTimer >= 1.0)
		{
			reportTimer = 0.0;

			TypedArray<Node2D> bodies = self->get_overlapping_bodies();
			Node2D* player = FindPlayer();

			if (player == nullptr)
			{
				Output("KILLZONE: line y = %.0f | overlapping = %d | "
					   "PLAYER NOT FOUND by name",
					   self->get_global_position().y, (int)bodies.size());
			}
			else
			{
				Output("KILLZONE: line y = %.0f | player y = %.0f | "
					   "overlapping = %d",
					   self->get_global_position().y,
					   player->get_global_position().y,
					   (int)bodies.size());
			}
		}
	}

	// ---- brief lock so one fall triggers exactly one respawn ----
	if (respawnCooldown > 0.0)
	{
		respawnCooldown -= delta;
		return;
	}

	// ---- she fell in: send her back to her last safe ground ----
	if (self->has_overlapping_bodies())
	{
		TypedArray<Node2D> bodies = self->get_overlapping_bodies();
		for (int i = 0; i < bodies.size(); i++)
		{
			Node* n = Object::cast_to<Node>(bodies[i]);
			if (n == nullptr) continue;

			if (n->has_method("respawn"))
			{
				Output("Killzone: fell in -- respawning at last safe ground.");
				n->call("respawn");
				respawnCooldown = RESPAWN_LOCK;
				break;
			}
		}
	}
}

// Script Block End
JENOVA_SCRIPT_END
