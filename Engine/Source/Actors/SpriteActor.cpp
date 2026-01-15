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

	collision = CreateComponent<CollisionComponent>(CollisionChannel::Player, CollisionChannel::Player);
}

Krampus::SpriteActor::SpriteActor(Level* _level, const RectangleShapeData& _data, bool test)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Actors);

	collision = CreateComponent<CollisionComponent>(CollisionChannel::Player, CollisionChannel::Player);
	if (test)
	{
		//physics = CreateComponent<PhysicsComponent>();
		collision->BindCollisionResolution();
		collision->onCollision.AddListener([this](CollisionInfo _info) {Debug::DrawDebugCircle(GetLevel(), _info.contactPoint, 30); });
		collision->BindCollisionResolution();
		M_INPUT.MouseMovedScreen.AddListener([this](FVector2 _pos) {transform.position = _pos; });
		//physics->BindCollisionResponse();
		//physics->AddImpulse(FVector2(0, -500), FVector2(100));
	}
}

void Krampus::SpriteActor::Tick(const float& _deltaTime)
{
	Actor::Tick(_deltaTime);

	//if (physics) physics->AddForce(FVector2(0, -50), FVector2(10, 0));
}