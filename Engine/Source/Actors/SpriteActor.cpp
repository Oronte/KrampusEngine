#include "SpriteActor.h"
#include "GameFramework/Level.h"

#include"Managers/InputManager.h"
#include"Managers/TimerManager.h"
#include "Graphics/Mouse.h"
#include "Bullet.h"

Krampus::SpriteActor::SpriteActor(Level* _level, const CircleShapeData& _data)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Actors);
	collision = CreateComponent<CollisionComponent>(CollisionChannel::All, CollisionChannel::All);
	//sprite->useDebug = true;

	//CreateComponent<CameraComponent>()->attachedToOwner = true;

	handle.push_back(GetWorld()->GetInputManager()->Z.onPerform.AddListener(this, &SpriteActor::MoveRight));
	handle.push_back(GetWorld()->GetInputManager()->S.onPerform.AddListener(this, &SpriteActor::MoveLeft));
	handle.push_back(GetWorld()->GetInputManager()->D.onPerform.AddListener(this, &SpriteActor::MoveDown));
	handle.push_back(GetWorld()->GetInputManager()->Q.onPerform.AddListener(this, &SpriteActor::MoveUp));
	handle.push_back(GetWorld()->GetInputManager()->MouseLeftClick.onPress.AddListener(this, &SpriteActor::Shoot));
	h = GetWorld()->GetInputManager()->MouseMovedWorld.AddListener(this, &SpriteActor::Rotate);
}

Krampus::SpriteActor::SpriteActor(Level* _level, const RectangleShapeData& _data)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	sprite->SetZOrder(ZOrder::Actors);
	collision = CreateComponent<CollisionComponent>(CollisionChannel::All, CollisionChannel::All);
	//sprite->useDebug = true;
}

void Krampus::SpriteActor::MoveUp()
{
	transform.position += transform.Up() * 200.0f * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void Krampus::SpriteActor::MoveRight()
{
	transform.position += transform.Right() * 200.0f * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void Krampus::SpriteActor::MoveDown()
{
	transform.position += transform.Down() * 200.0f * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void Krampus::SpriteActor::MoveLeft()
{
	transform.position += transform.Left() * 200.0f * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void Krampus::SpriteActor::Rotate(IVector2 _pos)
{
	transform.LookAt(_pos);
}

void Krampus::SpriteActor::Shoot()
{
	RectangleShapeData _data;
	_data.size = FVector2(50, 100);
	_data.texturePath = "Bullet";
	Actor* _actor = GetLevel()->SpawnActor<Bullet>(transform.Right(), _data);
	_actor->transform.position = transform.position + transform.Right() * 25;
	_actor->transform.rotation = transform.rotation + Angle(90, true);
}
