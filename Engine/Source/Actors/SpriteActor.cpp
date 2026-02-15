#include "SpriteActor.h"
#include "GameFramework/Level.h"

#include"Managers/InputManager.h"
#include"Managers/TimerManager.h"
#include "Graphics/Mouse.h"

Krampus::SpriteActor::SpriteActor(Level* _level, const CircleShapeData& _data)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Actors);
	collision = CreateComponent<CollisionComponent>(CollisionChannel::All, CollisionChannel::All);
}

Krampus::SpriteActor::SpriteActor(Level* _level, const RectangleShapeData& _data)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Actors);
	collision = CreateComponent<CollisionComponent>(CollisionChannel::All, CollisionChannel::All);
	//sprite->useDebug = true;
}