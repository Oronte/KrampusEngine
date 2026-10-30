#pragma once
#include "Component.h"
#include "Utilities/Debug/Color.h"
#include <string>

namespace Krampus
{
	/// <summary>The drawn symbol of a billboard without texture (saved as an int: never reorder).</summary>
	enum class BillboardIcon
	{
		Actor,		// a dot
		Camera,
		Sound,
		Light,
		Effect,		// particles, explosions
		Spawn,		// a flag: where something appears
		Logic,		// a gear: scripts, triggers, managers
		Star
	};

	/// <summary>
	/// A small icon on the actor in the editor's Viewport: click it to select the actor, handy for the ones nothing shows
	/// (empty actors, sounds, spawn points, effect emitters...). Never drawn in Play nor in the game: the editor draws it, the game
	/// does not know about it (it costs nothing there).
	/// </summary>
	class BillboardComponent : public Component
	{
	public:
		int				icon		= static_cast<int>(BillboardIcon::Actor);	// BillboardIcon, used without texture
		std::string		texture;			// "Folder/Name.png" in Content/Textures; empty: the icon
		float			size		= 32.f;	// pixels on the screen, whatever the zoom
		Color			tint		= Color(255, 255, 255, 255);

		BillboardComponent(Actor* _owner, const BillboardIcon& _icon = BillboardIcon::Actor)
			: Component(_owner), icon(static_cast<int>(_icon))
		{
			name = NAME_OF(BillboardComponent);
		}

		std::string ToString() const override { return name + " -> " + (texture.empty() ? "icon " + std::to_string(icon) : texture); }
	};
}
