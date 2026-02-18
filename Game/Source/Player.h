#pragma once
#include "Actors/Actor.h"
#include "Components/SpriteComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/InputComponent.h"

using namespace Krampus;

class Player : public Actor
{
	SpriteComponent* sprite = nullptr;
	CollisionComponent* collision = nullptr;
	InputComponent* inputs = nullptr;

	Event<CollisionInfo>::ListenerHandle handleEnter;
	Event<CollisionComponent*>::ListenerHandle handleExit;

public:
	Player(Level* _level);
	Player(Level* _level, CircleShapeData _data);
	Player(Level* _level, RectangleShapeData _data);

	//void MoveLeft();
	//void MoveRight();
	//void MoveUp();
	//void MoveDown();

	void SetPosition(FVector2 _pos)
	{
		transform.position = _pos;
	}
};

