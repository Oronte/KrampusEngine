#pragma once
#include "Actor.h"
#include "Components/CameraComponent.h"

namespace Krampus
{

	class CameraActor : public Actor
	{
	protected:
		CameraComponent*	camera	= nullptr;

	public:
		CameraActor(Level* _level);
	};

}


