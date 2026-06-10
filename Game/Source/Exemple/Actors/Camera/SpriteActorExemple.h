#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"

namespace Krampus::Exemple
{

	class SpriteActorExemple : public Actor
	{
	public:
		SpriteActorExemple(Level* _level, const RectangleShapeData& _data);
		SpriteActorExemple(Level* _level, const CircleShapeData& _data);
	};

}