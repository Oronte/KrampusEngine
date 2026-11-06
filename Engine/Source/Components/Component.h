#pragma once
#include "OwnedComponent.h"

namespace Krampus
{
	class Actor;

	/// <summary>
	/// Base class for the components of an actor (world space): behavior, physics, rendering...
	/// </summary>
	class Component : public OwnedComponent<Actor>
	{
	protected:
		explicit Component(Actor* _owner) : OwnedComponent(_owner) { name = NAME_OF(Component); }
	};
}
