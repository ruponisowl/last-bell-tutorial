/* Last Bell: Campus Rush -- the guard at the gate
 *
 * Two data structures meet here:
 *   StringQueue   (linked)   -- his dialogue lines, read once
 *   CircularQueue (array)    -- his thrown ID cards, reused forever
 */

#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/input.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/variant/variant.hpp>
#include <Godot/variant/vector2.hpp>
#include <Godot/variant/color.hpp>
#include <cstdio>
#include <cmath>

#include "D:/last-bell/scripts/string_queue.h"
#include "D:/last-bell/scripts/circular_queue.h"

using namespace godot;
using namespace jenova::sdk;

JENOVA_SCRIPT_BEGIN

// ------------------------------------------------------------------
//  Tuning
// ------------------------------------------------------------------
const double FIRE_INTERVAL = 1.20;    // seconds between thrown cards
const double SHOT_SPEED    = 110.0;   // pixels per second
const double SHOT_LIFE     = 2.00;    // seconds before a card expires
const double MUZZLE_UP     = 18.0;    // how high up his body cards leave
const double MUZZLE_OUT    = 22.0;    // how far in front

const int    GUARD_HEALTH  = 10;      // paper planes needed
const double FLASH_TIME    = 0.12;    // red flash when hit
const double LAST_WORDS    = 2.50;    // how long his final line stays up

const double PERCH_TIME    = 3.50;    // seconds on a platform before moving
const double JUMP_TIME     = 0.60;    // seconds in the air
const double ARC_HEIGHT    = 70.0;    // how high the hop arcs

// Spot 3 is his aerial phase: he paces and rains cards straight down,
// and she has no way to reach him. Pure dodging.
const int    RAIN_SPOT     = 2;       // 0-based, so this is GuardSpot3
const double SWAY_RANGE    = 150.0;    // how far he paces either way
const double SWAY_SPEED    = 1.1;     // radians per second
const double RAIN_INTERVAL = 0.35;    // cards fall faster than he throws

const double LEAVE_DIST  = 420.0;   // further than this and he stops fighting
const double RETURN_DIST = 260.0;   // closer than this and he starts again
const double PARKED = -99999.0;


// ------------------------------------------------------------------
//  States
// ------------------------------------------------------------------
const int STATE_WAITING  = 0;
const int STATE_TALKING  = 1;
const int STATE_FIGHTING = 2;
const int STATE_DYING    = 3;
const int STATE_DONE     = 4;
const int STATE_STANDBY  = 5;   // she died; he stands down until she returns

// ------------------------------------------------------------------
//  State
// ------------------------------------------------------------------
StringQueue   lines;
CircularQueue pool;

int    state      = STATE_WAITING;
int    health     = GUARD_HEALTH;
double fireTimer  = 0.0;
double flashTimer = 0.0;
double dyingTimer = 0.0;
Node*  lastScene  = nullptr;

// where he can perch, read from the GuardSpot markers in the level
Vector2 spots[4];
int     spotCount  = 0;
int     spotIndex  = 0;
double  perchTimer = 0.0;
double  jumpT      = -1.0;       // below zero means he is standing still
Vector2 jumpFrom;
Vector2 jumpTo;
double  swayTime   = 0.0;


void ParkEveryCard(Node* shots)
{
	if (shots == nullptr) return;

	for (int i = 0; i < shots->get_child_count(); i++)
	{
		Node2D* n = Object::cast_to<Node2D>(shots->get_child(i));
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
		lines.clear();
		pool.clear();
		state      = STATE_WAITING;
		health     = GUARD_HEALTH;
		fireTimer  = 0.0;
		flashTimer = 0.0;
		dyingTimer = 0.0;
		spotIndex  = 0;
		perchTimer = 0.0;
		jumpT      = -1.0;
		swayTime   = 0.0;

		// Read his perches out of the level. Missing ones are fine --
		// with fewer than two he simply never jumps.
		spotCount = 0;
		for (int i = 1; i <= 4; i++)
		{
			char name[20];
			snprintf(name, sizeof(name), "GuardSpot%d", i);

			Node2D* marker = Object::cast_to<Node2D>(
				scene->find_child(String(name), true, false));

			if (marker != nullptr)
			{
				spots[spotCount] = marker->get_global_position();
				spotCount++;
			}
		}

		char msg[64];
		snprintf(msg, sizeof(msg), "[GUARD] waiting -- %d perch(es) found", spotCount);
		Output(msg);
	}

	self->set_visible(true);
	self->set_modulate(Color(1, 1, 1));
	ParkEveryCard(self->get_node_or_null(NodePath("Shots")));
}


void OnProcess(Caller* instance, double delta)
{
	Area2D* self = GetSelf<Area2D>(instance);
	Node* scene = self->get_tree()->get_current_scene();
	Node* box = scene->find_child("DialogueBox", true, false);
	Node* shots = self->get_node_or_null(NodePath("Shots"));

	Input* input = Input::get_singleton();

	if (state == STATE_DONE) return;

	// ==============================================================
	//  DYING
	// ==============================================================
	if (state == STATE_DYING)
	{
		dyingTimer -= delta;
		if (dyingTimer <= 0.0)
		{
			state = STATE_DONE;
			if (box != nullptr) box->call("hide_box");
			self->set_visible(false);
			Node* moodOff = scene->find_child("Mood", true, false);
			if (moodOff != nullptr) moodOff->call("set_boss", false);
			Node* gate = scene->find_child("Gate", true, false);
			if (gate != nullptr)
			{
				gate->queue_free();
				Output("[GUARD] gate is open");
			}
		}
		return;
	}

	// ==============================================================
	//  WAITING
	// ==============================================================
	if (state == STATE_WAITING)
	{
		if (!self->has_overlapping_bodies()) return;

		lines.clear();
		lines.enqueue("GUARD:  Oi! Where do you think you're going?");
		lines.enqueue("GUARD:  No ID card, no entry. Rules are rules.");
		lines.enqueue("YOU:    I don't have my ID card.");
		lines.enqueue("YOU:    Class starts in five minutes!");
		lines.enqueue("GUARD:  Then you had better get past me.");

		state = STATE_TALKING;

		char msg[64];
		snprintf(msg, sizeof(msg), "[GUARD] %d lines enqueued", lines.size());
		Output(msg);

		if (box != nullptr && lines.peek() != nullptr)
			box->call("show_line", String(lines.peek()));

		return;
	}

	// ==============================================================
	//  TALKING
	// ==============================================================
	if (state == STATE_TALKING)
	{
		if (input == nullptr) return;
		if (!input->is_action_just_pressed("talk")) return;

		char spoken[MAX_LINE];
		lines.dequeue(spoken);

		if (lines.isEmpty())
		{
			state = STATE_FIGHTING;
			fireTimer  = 0.6;
			perchTimer = PERCH_TIME;
			if (box != nullptr) box->call("hide_box");
			Output("[GUARD] dialogue queue empty -- fight begins");
						Node* mood = scene->find_child("Mood", true, false);
			if (mood != nullptr) mood->call("set_boss", true);
			return;
		}
			Node* bar = scene->find_child("BossBar", true, false);
			if (bar != nullptr) bar->call("set_boss_health", health, GUARD_HEALTH);
		char msg[64];
		snprintf(msg, sizeof(msg), "[GUARD] %d lines left", lines.size());
		Output(msg);
		
		if (box != nullptr)
			box->call("show_line", String(lines.peek()));

		return;
	}

	// ==============================================================
	//  STAND DOWN / RE-ENGAGE
	// ==============================================================
	Node2D* her = Object::cast_to<Node2D>(
		self->get_tree()->get_first_node_in_group("player"));

	double gap = 0.0;
	if (her != nullptr)
		gap = fabs(her->get_global_position().x - self->get_global_position().x);

	if (state == STATE_STANDBY)
	{
		if (her != nullptr && gap < RETURN_DIST)
		{
			state      = STATE_FIGHTING;
			fireTimer  = 0.6;
			perchTimer = PERCH_TIME;
			Node* moodOn = scene->find_child("Mood", true, false);
			if (moodOn != nullptr) moodOn->call("set_boss", true);
						Node* barOn = scene->find_child("BossBar", true, false);
			if (barOn != nullptr) barOn->call("set_boss_health", health, GUARD_HEALTH);
			Output("[GUARD] she's back -- fight resumes");
		}
		return;
	}

	if (her != nullptr && gap > LEAVE_DIST)
	{
		state = STATE_STANDBY;
		ParkEveryCard(shots);
		pool.clear();
		jumpT     = -1.0;
		spotIndex = 0;
		if (spotCount > 0) self->set_global_position(spots[0]);
		self->set_modulate(Color(1, 1, 1));
		Node* moodOff = scene->find_child("Mood", true, false);
		if (moodOff != nullptr) moodOff->call("set_boss", false);
				Node* barGone = scene->find_child("BossBar", true, false);
		if (barGone != nullptr) barGone->call("hide_bar");
		Output("[GUARD] she's gone -- standing down");
		return;
	}
	// ==============================================================
	//  FIGHTING
	// ==============================================================

	// ---- did a paper plane land? ---------------------------------
	// We poll rather than being called: a Jenova C++ script does not
	// expose methods to GDScript the way a .gd file does.
	TypedArray<Area2D> hits = self->get_overlapping_areas();
	for (int i = 0; i < hits.size(); i++)
	{
		Node* n = Object::cast_to<Node>(hits[i]);
		if (n == nullptr) continue;
		if (!n->is_in_group("plane")) continue;

				health--;
		flashTimer = FLASH_TIME;
		n->queue_free();

		Node* hitSfx = self->get_node_or_null(NodePath("Hit"));
		if (hitSfx != nullptr) hitSfx->call("play");

		char msg[64];
		snprintf(msg, sizeof(msg), "[GUARD] hit -- health %d", health);
		Output(msg);
				Node* barHit = scene->find_child("BossBar", true, false);
		if (barHit != nullptr) barHit->call("set_boss_health", health, GUARD_HEALTH);

		if (health <= 0)
		{
			state = STATE_DYING;
			dyingTimer = LAST_WORDS;
			self->set_modulate(Color(1, 1, 1));
			ParkEveryCard(shots);
			pool.clear();

			if (box != nullptr)
				box->call("show_line",
					String("GUARD:  Fine... go. Don't tell anyone I let you."));

			
			Output("[GUARD] down");
						Node* moodOff = scene->find_child("Mood", true, false);
			if (moodOff != nullptr) moodOff->call("set_boss", false);
						Node* barOff = scene->find_child("BossBar", true, false);
			if (barOff != nullptr) barOff->call("hide_bar");
			return;
		}
	}

	// ---- red flash ------------------------------------------------
	if (flashTimer > 0.0)
	{
		flashTimer -= delta;
		self->set_modulate(Color(1.0f, 0.4f, 0.4f));
		if (flashTimer <= 0.0) self->set_modulate(Color(1, 1, 1));
	}

	// ---- hop between perches ---------------------------------------
	bool airborne = false;

	if (spotCount >= 2)
	{
		if (jumpT >= 0.0)
		{
			airborne = true;
			jumpT += delta / JUMP_TIME;

			if (jumpT >= 1.0)
			{
				jumpT = -1.0;
				airborne = false;
				self->set_global_position(jumpTo);
				perchTimer = PERCH_TIME;
			}
			else
			{
				// Straight line from A to B, lifted by a parabola.
				// 4t(1-t) peaks at 1.0 when t = 0.5 and is 0 at both ends,
				// so he leaves and lands cleanly.
				Vector2 p = jumpFrom.lerp(jumpTo, (float)jumpT);
				p.y -= (float)(ARC_HEIGHT * 4.0 * jumpT * (1.0 - jumpT));
				self->set_global_position(p);
			}
		}
		else
		{
			perchTimer -= delta;
			if (perchTimer <= 0.0)
			{
				jumpFrom  = self->get_global_position();
				spotIndex = (spotIndex + 1) % spotCount;
				jumpTo    = spots[spotIndex];
				jumpT     = 0.0;
				airborne  = true;
			}
		}
	}

	// ---- aerial phase: pace side to side ---------------------------
	bool raining = (spotCount > RAIN_SPOT
				 && spotIndex == RAIN_SPOT
				 && jumpT < 0.0);

	if (raining)
	{
		swayTime += delta;
		Vector2 base = spots[RAIN_SPOT];
				// Triangle wave: constant speed back and forth, so dropped
		// cards land evenly spaced instead of bunching at the ends.
		double phase = fmod(swayTime * SWAY_SPEED / 6.2831853, 1.0);
		double sway  = (4.0 * fabs(phase - 0.5) - 1.0) * SWAY_RANGE;
		self->set_global_position(Vector2((float)(base.x + sway), base.y));
	}
	else
	{
		swayTime = 0.0;
	}

	// ---- aim -------------------------------------------------------
	Node* playerNode = self->get_tree()->get_first_node_in_group("player");
	Node2D* player = Object::cast_to<Node2D>(playerNode);

	Vector2 here = self->get_global_position();
	double facing = -1.0;
	if (player != nullptr && player->get_global_position().x > here.x)
		facing = 1.0;

	// ---- throw a card on the timer ---------------------------------
	// He does not throw mid-hop. That gap is her window to close in.
	if (!airborne)
	{
		fireTimer -= delta;
		if (fireTimer <= 0.0)
		{
			fireTimer = raining ? RAIN_INTERVAL : FIRE_INTERVAL;

			Shot s;
			if (raining)
			{
				// Dropped from his boots, straight down.
				s.x  = here.x;
				s.y  = here.y;
				s.vx = 0.0;
				s.vy = SHOT_SPEED;
			}
			else
			{
				s.x  = here.x + facing * MUZZLE_OUT;
				s.y  = here.y - MUZZLE_UP;
				s.vx = facing * SHOT_SPEED;
				s.vy = 0.0;
			}
			s.age = 0.0;
			s.alive = true;

			if (!pool.enqueue(s))
			{
				// OVERFLOW: the ring is full, so this shot is skipped.
				// The pool doubles as a rate limiter.
				Output("[POOL] full -- shot skipped (overflow)");
			}
		}
	}

	// ---- move every live card --------------------------------------
	if (shots != nullptr)
	{
		int childCount = shots->get_child_count();

		for (int i = 0; i < pool.size(); i++)
		{
			Shot* s = pool.peekAt(i);
			int slot = pool.slotOf(i);
			if (s == nullptr || slot < 0 || slot >= childCount) continue;

			Node2D* node = Object::cast_to<Node2D>(shots->get_child(slot));
			if (node == nullptr) continue;

			if (!s->alive)
			{
				node->set_visible(false);
				node->set_global_position(Vector2(PARKED, PARKED));
				continue;
			}

			s->x   += s->vx * delta;
			s->y   += s->vy * delta;
			s->age += delta;

			if (s->age > SHOT_LIFE)
			{
				s->alive = false;
				node->set_visible(false);
				node->set_global_position(Vector2(PARKED, PARKED));
				continue;
			}

			node->set_visible(true);
			node->set_global_position(Vector2(s->x, s->y));

			Area2D* card = Object::cast_to<Area2D>(node);
			if (card != nullptr && card->has_overlapping_bodies())
			{
				if (playerNode != nullptr)
					playerNode->call("take_damage", 1);

				s->alive = false;
				node->set_visible(false);
				node->set_global_position(Vector2(PARKED, PARKED));
			}
		}
	}

	// ---- reclaim dead cards from the front -------------------------
	// A queue only removes from the front, so a card destroyed in
	// mid-flight waits its turn. This keeps it a true FIFO rather
	// than a list with holes in it.
	while (!pool.isEmpty())
	{
		Shot* oldest = pool.peekAt(0);
		if (oldest == nullptr || oldest->alive) break;
		pool.dequeue(nullptr);
	}
}

JENOVA_SCRIPT_END
