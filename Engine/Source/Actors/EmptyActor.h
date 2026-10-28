#pragma once
#include "Actor.h"
#include "Reflection/Reflection.h"

namespace Krampus
{

	/// <summary>
	/// Actor without behavior: a transform that other actors can be attached to. Can be added from the editor Hierarchy.
	/// </summary>
	class EmptyActor : public Actor
	{
		KACTOR(EmptyActor, Actor)

	public:
		/// <summary>
		/// Constructs an EmptyActor within the given level.
		/// </summary>
		/// <param name="_level">Pointer to the owning Level.</param>
		EmptyActor(Level* _level)
			: Actor(_level)
		{
			name = "EmptyActor";
		}
	};

}
