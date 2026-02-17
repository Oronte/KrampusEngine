#include "Player.h"
#include "Managers/InputManager.h"
#include "BloodFX.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"

Player::Player(Level* _level)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(1024.0f, 559.0f) * 0.3f, "KrampusEngineBanner"));
	collision = CreateComponent<CollisionComponent>(CollisionChannel::All, CollisionChannel::All);
	collision->BindCollisionResolution();
	collision->useDebug = true;

	inputs = CreateComponent<InputComponent>();
	inputs->Bind(&GetWorld()->GetInputManager()->D.onPerform, this, &Player::MoveRight);
	inputs->Bind(&GetWorld()->GetInputManager()->Q.onPerform, this, &Player::MoveLeft);
	inputs->Bind(&GetWorld()->GetInputManager()->Z.onPerform, this, &Player::MoveUp);
	inputs->Bind(&GetWorld()->GetInputManager()->S.onPerform, this, &Player::MoveDown);
}

Player::Player(Level* _level, CircleShapeData _data)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	collision = CreateComponent<CollisionComponent>(CollisionChannel::All, CollisionChannel::All);
	collision->useDebug = true;
}

Player::Player(Level* _level, RectangleShapeData _data)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(_data);
	collision = CreateComponent<CollisionComponent>(CollisionChannel::All, CollisionChannel::All);
	collision->useDebug = true;
}

void Player::MoveLeft()
{
	transform.position += FVector2(-200.0f, 0) * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void Player::MoveRight()
{
	transform.position += FVector2(200.0f, 0) * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void Player::MoveUp()
{
	transform.position += FVector2(0, -200.0f) * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void Player::MoveDown()
{
	transform.position += FVector2(0, 200.0f) * GetWorld()->GetTimerManager()->GetDeltaTime();
}
