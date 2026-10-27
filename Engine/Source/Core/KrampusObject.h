#pragma once
#include "Engine.h"

namespace Krampus
{
	/// <summary>
	/// Base class for objects that belong to and have access to the Krampus engine world.
	/// </summary>
	class KrampusObject
	{
		Engine*		world	= nullptr;

	public:
		/// <summary>
		/// Gets a pointer to the engine world instance.
		/// </summary>
		/// <returns>Pointer to the Engine world.</returns>
		inline Engine* GetWorld() const
		{
			return world;
		}

		/// <summary>
		/// Constructs a KrampusObject associated with the specified engine instance.
		/// </summary>
		/// <param name="_engine">Pointer to the Engine instance.</param>
		KrampusObject(Engine* _engine)
			: world(_engine) { }
	};
}
