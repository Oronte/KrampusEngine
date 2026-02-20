#include "Player.h"
#include "Managers/InputManager.h"
#include "BloodFX.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"

Player::Player(Level* _level)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(150.0f, 150.0f), "Character/JumpPack_CharacterSpriteSheet"));
	sprite->useDebug = true;
	collision = CreateComponent<CollisionComponent>(CollisionChannel::All, CollisionChannel::All);

	inputs = CreateComponent<InputComponent>();
	inputs->Bind(&GetWorld()->GetInputManager()->D.onPerform, this, &Player::MoveRight);
	inputs->Bind(&GetWorld()->GetInputManager()->Q.onPerform, this, &Player::MoveLeft);
	inputs->Bind(&GetWorld()->GetInputManager()->Z.onPerform, this, &Player::MoveUp);
	inputs->Bind(&GetWorld()->GetInputManager()->S.onPerform, this, &Player::MoveDown);

	animation = CreateComponent<AnimationComponent>();

	animation->AddAnimation("Idle", AnimationData(4, 0.5f, SpriteData(IVector2(0, 32 * 0), IVector2(32))));
	animation->AddAnimation("Move", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 1), IVector2(32))));
	animation->AddAnimation("Dash", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 3), IVector2(32))));
	animation->AddAnimation("Hit", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 4), IVector2(32)), false));
	animation->AddAnimation("Death", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 5), IVector2(32)), false));
	animation->AddAnimation("Sleep", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 6), IVector2(32))));

	animation->StartAnimation();
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
	GetLevel()->SpawnActor<BloodFX>()->transform.position = transform.position;
}
