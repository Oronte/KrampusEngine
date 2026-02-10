#pragma once
#include "Actors/SpriteActor.h"

class Bullet : public Krampus::SpriteActor
{
	Krampus::FVector2 direction;

public:
	Bullet(Krampus::Level* _level, Krampus::FVector2 _direction, Krampus::RectangleShapeData _data)
		: Krampus::SpriteActor(_level, _data)
	{
		direction = _direction;
	}

	virtual void Tick(const float& _deltaTime) override
	{
		Super::Tick(_deltaTime);

		transform.position += direction * 500.0f * _deltaTime;
	}
};

