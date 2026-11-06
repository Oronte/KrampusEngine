#pragma once
#include "Reflection/Reflection.h"
#include "Components/SpriteComponent.h"

namespace Krampus
{

	/// <summary>
	/// A wall or a crate of Blackout: a plain sprite (its size is the wall's size), a static body that stops the characters and
	/// the bullets, and a shadow caster that stops the flashlight. Place, scale and resize it in the editor.
	/// </summary>
	class BlackoutWall : public Actor
	{
		KACTOR(BlackoutWall, Actor)

	public:
		BlackoutWall(Level* _level);
	};

	/// <summary>
	/// The floor of Blackout: a tiled picture (Content/Textures/Blackout/Floor.png) behind everything, without a body.
	/// </summary>
	class BlackoutFloor : public Actor
	{
		KACTOR(BlackoutFloor, Actor)

	public:
		BlackoutFloor(Level* _level);
	};

}
