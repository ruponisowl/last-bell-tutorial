/* =====================================================================
   CG Coin  --  Last Bell: Campus Rush
   Jenova C++ Script. Attach to the coin's Area2D.

   What this does:
	 - every coin registers itself in ONE shared linked list
	 - touching a coin runs a SEARCH, then an UPDATE on that record
	 - the CG counter in the corner is redrawn from the list
	 - the coin hides
	 - touching it again gives nothing, because the list remembers

   Delete coin_linked_list.h and none of this compiles. That is the
   point: the data structure is not decoration, it runs the game.

   Scene setup this expects:
	 Coin  : Area2D, Mask includes the player's layer
	 Label : named exactly "CGLabel", anywhere under a CanvasLayer
   ===================================================================== */

// Godot SDK
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/label.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/classes/audio_stream_player2d.hpp>

// Jenova SDK
#include <JenovaSDK.h>

// MY DATA STRUCTURE
#include "D:/last-bell/scene/coin_linked_list.h"

#include <cstdio>   // snprintf, for formatting the label text

using namespace godot;
using namespace jenova::sdk;

JENOVA_CLASS_NAME("CG Coin")

// ---------------------------------------------------------------------
//  SHARED STATE
//
//  Anything declared here is shared by every node running this script.
//  That was a bug for a per-coin "collected" flag. Here it is exactly
//  right: one list, one score, every coin talking to the same records.
// ---------------------------------------------------------------------
CoinLinkedList coinList;        // the structure itself
int   nextCoinId = 101;         // ids count up from 101
int   cgPoints   = 0;           // score in TENTHS: 40 means CG 4.00
Node* lastScene  = nullptr;     // used to notice a level reload

const int COIN_VALUE   = 1;     // each coin is 0.1 CG
const int COIN_SECTION = 1;     // 1 = parking lot
const int CG_TARGET    = 40;    // 40 tenths = CG 4.00

// Prints the whole list on every pickup. Brilliant for your viva demo,
// noisy the rest of the time -- set false when you get bored of it.
const bool SHOW_LIST_ON_PICKUP = true;


// Script Block Begin
JENOVA_SCRIPT_BEGIN

// ---------------------------------------------------------------------
//  Redraw the counter from the list.
//  Finds the Label by NAME, so there's no fragile node path to keep
//  in sync. Name your Label exactly "CGLabel" and it just works.
// ---------------------------------------------------------------------
void UpdateCGLabel()
{
	Node* scene = GetTree()->get_current_scene();
	if (scene == nullptr) return;

	Node* found = scene->find_child("CGLabel", true, false);
	Label* label = Object::cast_to<Label>(found);
	if (label == nullptr) return;     // no label in this scene, fine

	char buf[48];
	snprintf(buf, sizeof(buf), "CG %.2f / 4.00", cgPoints / 10.0);
	label->set_text(String(buf));
}

// ---------------------------------------------------------------------
//  OnReady -- each coin REGISTERS ITSELF in the shared list
// ---------------------------------------------------------------------
void OnReady(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);
	if (self == nullptr) return;

	// Has the level just (re)loaded? Then throw away the old records
	// before anyone registers. This is the CLEAR operation.
	//
	// *** DEATH HANDLING GOES HERE LATER ***
	// Whatever you decide death does, this is where progress resets.
	Node* scene = GetTree()->get_current_scene();
	if (scene != lastScene)
	{
		coinList.clear();        // frees every node, no leaks
		nextCoinId = 101;
		cgPoints   = 0;
		lastScene  = scene;
		Output("Coin list cleared for new level.");
	}

	// Give this coin an id, and store it ON THE NODE.
	// A script variable would be shared, so all coins would end up
	// with the same id. set_meta attaches it to this one node.
	int myId = nextCoinId++;
	self->set_meta("coin_id", myId);

	// THE INSERT OPERATION
	Vector2 pos = self->get_position();
	if (!coinList.insert(myId, COIN_SECTION, COIN_VALUE, pos.x, pos.y))
	{
		Output("Coin %d: insert rejected, duplicate id.", myId);
	}

	self->show();
	self->set_monitoring(true);

	UpdateCGLabel();
}

// ---------------------------------------------------------------------
//  OnProcess -- runs every frame
// ---------------------------------------------------------------------
void OnProcess(Caller* instance, double delta)
{
	Area2D* self = GetSelf<Area2D>(instance);
	if (self == nullptr) return;

	// A hidden coin is a collected coin. The state lives on the node,
	// because a script variable would be shared by all of them.
	if (!self->is_visible()) return;

	// Nothing touching me this frame
	if (!self->has_overlapping_bodies()) return;

	if (!self->has_meta("coin_id")) return;
	int myId = (int)(int64_t)self->get_meta("coin_id");

	// =================================================================
	//  THE DATA STRUCTURE DOING THE WORK
	//  collect() walks the chain looking for myId  -- O(n) SEARCH
	//  then flips that node's collected flag       -- O(1) UPDATE
	// =================================================================
	CollectResult result = coinList.collect(myId);

	switch (result)
	{
		case CollectResult::Success:
		{
			 cgPoints += COIN_VALUE;

			self->hide();
			self->set_monitoring(false);
			UpdateCGLabel();

			// play the pickup sound
			Node* sfxNode = self->get_node_or_null(NodePath("Pickup"));
			AudioStreamPlayer2D* sfx = Object::cast_to<AudioStreamPlayer2D>(sfxNode);
			if (sfx != nullptr)
			{
				sfx->play();
			}

			Output("Coin %d collected.  CG %.2f  (%d still out there)",
				   myId, cgPoints / 10.0,
				   coinList.remainingInSection(COIN_SECTION));

			if (SHOW_LIST_ON_PICKUP)
			{
				coinList.traverse();     // watch the records change
			}

			if (cgPoints >= CG_TARGET)
			{
				Output("CG 4.00 reached. You made it to class!");
				// *** WIN HANDLING GOES HERE LATER ***
			}
			break;
		}

		case CollectResult::AlreadyCollected:
			// The entire reason the list exists. Without it, walking
			// back over a coin would hand out free CG forever.
			Output("Coin %d already collected -- no points.", myId);
			self->hide();
			break;

		case CollectResult::NotFound:
			// A sprite with no record behind it means registration
			// failed in OnReady. Real bug, so say so loudly.
			Output("Coin %d has NO RECORD in the list. Check OnReady.", myId);
			break;
	}
}

// Script Block End
JENOVA_SCRIPT_END
