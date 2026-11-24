/* Last Bell: Campus Rush -- the elevator shaft
 *
 * Spiders drop from the ceiling while she climbs. They come from the
 * same CircularQueue used for the guard's ID cards: a fixed ring of
 * reused nodes, no allocation during play.
 */

#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/variant/variant.hpp>
#include <Godot/variant/vector2.hpp>
#include <cstdio>

#include "D:/last-bell/scripts/circular_queue.h"

using namespace godot;
using namespace jenova::sdk;

JENOVA_SCRIPT_BEGIN

// ------------------------------------------------------------------
//  Tuning
// ------------------------------------------------------------------
const double SPAWN_INTERVAL = 0.90;   // seconds between spiders
const double FALL_SPEED     = 130.0;  // pixels per second
const double SPIDER_LIFE    = 6.500;   // seconds before it despawns
const double SPAWN_HALF     = 120.0;  // how wide the band is, either side

const double PARKED = -99999.0;

// ------------------------------------------------------------------
//  State
// ------------------------------------------------------------------
CircularQueue web;              // the falling spiders

bool   active     = false;      // true while she is inside the shaft
double spawnTimer = 0.0;
Node*  lastScene  = nullptr;

// A tiny linear congruential generator, so the spread does not depend
// on anything outside this file. Same sequence every run, which makes
// the section testable rather than unfair.
unsigned int seed = 2463534242u;

double NextRandom()
{
	seed = seed * 1103515245u + 12345u;
	return (double)((seed >> 16) & 0x7FFF) / 32767.0;   // 0.0 .. 1.0
}


void ParkEverySpider(Node* nest)
{
	if (nest == nullptr) return;

	for (int i = 0; i < nest->get_child_count(); i++)
	{
		Node2D* n = Object::cast_to<Node2D>(nest->get_child(i));
		if (n == nullptr) continue;
		n->set_visible(false);
		n->set_global_position(Vector2(PARKED, PARKED));
	}
}


void OnReady(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);
	Node* scene = self->get_tree()->get_current_scene();

	if (scene != lastScene)
	{
		lastScene  = scene;
		web.clear();
		active     = false;
		spawnTimer = 0.0;
		seed       = 2463534242u;
		Output("[SHAFT] quiet");
	}

	ParkEverySpider(self->get_node_or_null(NodePath("Spiders")));
}


void OnProcess(Caller* instance, double delta)
{
	Area2D* self = GetSelf<Area2D>(instance);
	Node* nest = self->get_node_or_null(NodePath("Spiders"));
	if (nest == nullptr) return;

	Node* playerNode = self->get_tree()->get_first_node_in_group("player");

	// ---- is she in the shaft? --------------------------------------
	bool inside = self->has_overlapping_bodies();

	if (inside && !active)
	{
		active = true;
		spawnTimer = 0.5;
		Output("[SHAFT] she is climbing -- spiders waking");
			Node* mood = self->get_tree()->get_current_scene()
			->find_child("Mood", true, false);
		if (mood != nullptr) mood->call("set_shaft", true);
	}
	else if (!inside && active)
	{
		active = false;
		Output("[SHAFT] clear");
			Node* moodOff = self->get_tree()->get_current_scene()
			->find_child("Mood", true, false);
		if (moodOff != nullptr) moodOff->call("set_shaft", false);
	}

	// ---- drop a new spider -----------------------------------------
	Node2D* line = Object::cast_to<Node2D>(
		self->get_node_or_null(NodePath("SpawnLine")));

	if (active)
	{
		spawnTimer -= delta;
		if (spawnTimer <= 0.0 && line != nullptr)
		{
			spawnTimer = SPAWN_INTERVAL;

			Vector2 top = line->get_global_position();
			double spread = (NextRandom() * 2.0 - 1.0) * SPAWN_HALF;

			Shot s;
			s.x  = top.x + spread;
			s.y  = top.y;
			s.vx = 0.0;
			s.vy = FALL_SPEED;
			s.age = 0.0;
			s.alive = true;

			if (!web.enqueue(s))
			{
				// OVERFLOW: the ring is full, so this spider does not
				// appear. The pool limits how thick the rain can get.
				Output("[WEB] full -- spider skipped (overflow)");
			}
		}
	}

	// ---- move every live spider ------------------------------------
	int childCount = nest->get_child_count();

	for (int i = 0; i < web.size(); i++)
	{
		Shot* s = web.peekAt(i);
		int slot = web.slotOf(i);
		if (s == nullptr || slot < 0 || slot >= childCount) continue;

		Node2D* node = Object::cast_to<Node2D>(nest->get_child(slot));
		if (node == nullptr) continue;

		if (!s->alive)
		{
			node->set_visible(false);
			node->set_global_position(Vector2(PARKED, PARKED));
			continue;
		}

		s->y   += s->vy * delta;
		s->age += delta;

		if (s->age > SPIDER_LIFE)
		{
			s->alive = false;
			node->set_visible(false);
			node->set_global_position(Vector2(PARKED, PARKED));
			continue;
		}

		node->set_visible(true);
		node->set_global_position(Vector2(s->x, s->y));

		// The web: one line from the spider back up to the ceiling,
		// stretching as it descends. Called by name so this file does
		// not need the Line2D header.
		Node* thread = node->get_node_or_null(NodePath("Thread"));
		if (thread != nullptr && line != nullptr)
		{
			double up = line->get_global_position().y - s->y;
			thread->call("set_point_position", 1, Vector2(0.0f, (float)up));
		}

		Area2D* bug = Object::cast_to<Area2D>(node);
		if (bug != nullptr && bug->has_overlapping_bodies())
		{
			if (playerNode != nullptr)
				playerNode->call("take_damage", 1);

			s->alive = false;
			node->set_visible(false);
			node->set_global_position(Vector2(PARKED, PARKED));
		}
	}

	// ---- reclaim the dead from the front ---------------------------
	while (!web.isEmpty())
	{
		Shot* oldest = web.peekAt(0);
		if (oldest == nullptr || oldest->alive) break;
		web.dequeue(nullptr);
	}
}

JENOVA_SCRIPT_END
