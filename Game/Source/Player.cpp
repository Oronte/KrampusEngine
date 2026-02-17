#include "Player.h"
#include "Managers/InputManager.h"
#include "BloodFX.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"

Player::Player(Level* _level)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(100.0f, 100.0f), "Character/JumpPack_CharacterSpriteSheet"));
	collision = CreateComponent<CollisionComponent>();
	//camera = CreateComponent<CameraComponent>();
	animation = CreateComponent<AnimationComponent>();

	animation->AddAnimation("Idle", AnimationData(4, 0.5f, SpriteData(IVector2(0, 32 * 0), IVector2(32))));
	animation->AddAnimation("Move", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 1), IVector2(32))));
	animation->AddAnimation("Dash", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 3), IVector2(32))));
	animation->AddAnimation("Hit", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 4), IVector2(32)), false));
	animation->AddAnimation("Death", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 5), IVector2(32)), false));
	animation->AddAnimation("Sleep", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 6), IVector2(32))));

	animation->StartAnimation();

	inputs = CreateComponent<InputComponent>();
	inputs->Bind(&GetWorld()->GetInputManager()->D.onPerform, this, &Player::MoveRight);
	inputs->Bind(&GetWorld()->GetInputManager()->Q.onPerform, this, &Player::MoveLeft);
	inputs->Bind(&GetWorld()->GetInputManager()->D.onRelease, this, &Player::Idle);
	inputs->Bind(&GetWorld()->GetInputManager()->Q.onRelease, this, &Player::Idle);
	inputs->Bind(&GetWorld()->GetInputManager()->LControl.onPress, this, &Player::Sleep);
	inputs->Bind(&GetWorld()->GetInputManager()->H.onPress, this, &Player::Hit);
}

void Player::MoveLeft()
{
	transform.position += FVector2(-200.0f, 0) * GetWorld()->GetTimerManager()->GetDeltaTime();
	if (state == State::Idle)
	{
		Move();
		state = State::Moving;
	}
	transform.scale = FVector2(-1, 1);
}

void Player::MoveRight()
{
	transform.position += FVector2(200.0f, 0) * GetWorld()->GetTimerManager()->GetDeltaTime();
	if (state == State::Idle)
	{
		Move();
		state = State::Moving;
	}
	transform.scale = FVector2(1, 1);
}

void Player::Idle()
{
	state = State::Idle;
	animation->SetCurrentAnimation("Idle");
	animation->StartAnimation();
}

void Player::Move()
{
	animation->SetCurrentAnimation("Move");
	animation->StartAnimation();
}

void Player::Dash()
{
	animation->SetCurrentAnimation("Dash");
	animation->StartAnimation();
}

void Player::Hit()
{
	animation->SetCurrentAnimation("Hit");
	animation->StartAnimation();
	GetLevel()->SpawnActor<BloodFX>()->transform.position = transform.position - FVector2(50.0f, 50.0f);
}

void Player::Death()
{
	animation->SetCurrentAnimation("Death");
	animation->StartAnimation();
}

void Player::Sleep()
{
	animation->SetCurrentAnimation("Sleep");
	animation->StartAnimation();
}
