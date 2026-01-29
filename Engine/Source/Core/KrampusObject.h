#pragma once
#include "Engine.h"

namespace Krampus
{
	class KrampusObject
	{
		Engine* world = nullptr;

	public:
		inline Engine* GetWorld() const
		{
			return world;
		}

		KrampusObject(Engine* _engine)
			: world(_engine) { }
	};
}