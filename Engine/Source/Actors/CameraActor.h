#pragma once
#include "Actor.h"
#include "Components/CameraComponent.h"
#include "Reflection/Reflection.h"

namespace Krampus
{

	/// <summary>
	/// Actor that represents a camera viewpoint in the level scene.
	/// </summary>
	class CameraActor : public Actor
	{
		KACTOR(CameraActor, Actor)

	protected:
		CameraComponent*	camera	= nullptr;

	public:
		/// <summary>
		/// Constructs a CameraActor within the given level and initializes its camera component.
		/// </summary>
		/// <param name="_level">Pointer to the owning Level.</param>
		CameraActor(Level* _level);
	};

}
