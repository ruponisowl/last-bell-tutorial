/* =====================================================================
   CG Coin  --  Last Bell: Campus Rush
   Jenova C++ Script. Attach to an Area2D.

   Collision setup this relies on:
	 Player : Layer 2
	 Coin   : Mask  2      <- so the coin only ever sees the player

   Because of that, anything overlapping this coin IS the player,
   and we don't need to look the player up at all.
   ===================================================================== */

// Godot SDK
#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/node2d.hpp>
#include <Godot/classes/area2d.hpp>

// Jenova SDK
#include <JenovaSDK.h>

// Namespaces
using namespace godot;
using namespace jenova::sdk;

// Name shown in the Godot editor
JENOVA_CLASS_NAME("CG Coin")

// Script Block Begin
JENOVA_SCRIPT_BEGIN

// ---------------------------------------------------------------------
//  OnReady -- runs once when this coin enters the scene
// ---------------------------------------------------------------------
void OnReady(Caller* instance)
{
	Area2D* self = GetSelf<Area2D>(instance);
	if (self == nullptr) return;

	// Make sure the coin starts visible and detecting, even if we
	// hid it in a previous run.
	self->show();
	self->set_monitoring(true);
}

// ---------------------------------------------------------------------
//  OnProcess -- runs every frame
// ---------------------------------------------------------------------
void OnProcess(Caller* instance, double delta)
{
	// GetSelf gives us THIS coin. Twenty coins in the level means
	// twenty different "self" pointers, even though there is only
	// one copy of this script.
	Area2D* self = GetSelf<Area2D>(instance);
	if (self == nullptr) return;

	// THE PER-COIN FLAG.
	// We can't use a script variable to remember "already collected" --
	// in Jenova that variable is shared by every coin, so collecting
	// one would mark them all. Instead the state lives on the node:
	// a hidden coin is a collected coin.
	if (!self->is_visible()) return;

	// Is anything inside my collision shape right now?
	// Thanks to the collision mask, the only thing that can be
	// inside me is the player.
	if (self->has_overlapping_bodies())
	{
		self->hide();                 // the coin vanishes
		self->set_monitoring(false);  // stop detecting, costs nothing now

		Output("CG coin collected!");

		// LATER: your linked list goes here --
		//   CollectResult r = coinList.collect(myCoinId);
		//   if (r == CollectResult::Success) cgPoints += 1;
	}
}

// Script Block End
JENOVA_SCRIPT_END
