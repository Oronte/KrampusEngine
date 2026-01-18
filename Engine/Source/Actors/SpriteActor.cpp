#include "SpriteActor.h"
#include "GameFramework/Level.h"

#include"Managers/InputManager.h"
#include"Managers/TimerManager.h"
#include "Graphics/Mouse.h"

Krampus::SpriteActor::SpriteActor(Level* _level, const CircleShapeData& _data, bool test)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Actors);

	collision = CreateComponent<CollisionComponent>(CollisionChannel::Player, CollisionChannel::Player);

	if (test)
	{
		//collision->onCollision.AddListener([&](CollisionInfo _info) {
		//	Debug::DrawDebugCircle(GetLevel(), _info.avrageContactPoint, 30);
		//	for (FVector2 _point : _info.contacts)
		//	{
		//		Debug::DrawDebugCircle(level, _point, 30);
		//	}
		//	LOG_MSG(std::to_string(_info.contacts.size())); });
		//	M_INPUT.MouseMovedScreen.AddListener([this](FVector2 _pos) {transform.position = _pos; });
	
			physics = CreateComponent<PhysicsComponent>();
			physics->BindCollisionResponse();
			//physics->AddImpulse(FVector2(100, -500), FVector2(100));
	}
}

Krampus::SpriteActor::SpriteActor(Level* _level, const RectangleShapeData& _data, bool test)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Actors);

	collision = CreateComponent<CollisionComponent>(CollisionChannel::Player, CollisionChannel::Player);
	if (test)
	{
		physics = CreateComponent<PhysicsComponent>();
		//collision->BindCollisionResolution();
		//collision->onCollision.AddListener([&](CollisionInfo _info) {
		//	Debug::DrawDebugCircle(GetLevel(), _info.avrageContactPoint, 30);
		//	for (FVector2 _point : _info.contacts)
		//	{
		//		Debug::DrawDebugCircle(level, _point, 30);
		//	} 
		//	LOG_MSG(std::to_string(_info.contacts.size())); });
		//M_INPUT.MouseMovedScreen.AddListener([this](FVector2 _pos) {transform.position = _pos; });
		physics->BindCollisionResponse();
	}
}

void Krampus::SpriteActor::Tick(const float& _deltaTime)
{
	Actor::Tick(_deltaTime);

	//if (physics) physics->AddForce(FVector2(100, -50), FVector2(10, 0));
}