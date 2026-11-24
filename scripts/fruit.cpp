/* Last Bell: Campus Rush -- healing fruit */

#include <Godot/godot.hpp>
#include <Godot/classes/node.hpp>
#include <Godot/classes/area2d.hpp>
#include <Godot/classes/scene_tree.hpp>
#include <Godot/variant/variant.hpp>

using namespace godot;
using namespace jenova::sdk;

JENOVA_SCRIPT_BEGIN

void OnProcess(Caller* instance, double delta)
{
	Area2D* self = GetSelf<Area2D>(instance);

	// Hidden means already eaten. Using visibility as the flag keeps
	// every fruit independent, the same trick coin.cpp uses -- Jenova
	// shares plain variables across all nodes running this script.
	if (!self->is_visible()) return;

	if (!self->has_overlapping_bodies()) return;

	Node* player = self->get_tree()->get_first_node_in_group("player");
	if (player == nullptr) return;

	// She refuses the fruit on full hearts, so it stays put.
	bool eaten = (bool)player->call("heal", 1);
	if (!eaten) return;

	Output("[FRUIT] eaten -- one heart back");
	self->set_visible(false);
}

JENOVA_SCRIPT_END
