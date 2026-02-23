#include "CustomActor.h"
#include "Managers/InputManager.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"

using namespace Krampus;

CustomActor::CustomActor(Level* _level)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(150.0f, 150.0f), "Character/JumpPack_CharacterSpriteSheet"));
	collision = CreateComponent<CollisionComponent>(CollisionChannel::All, CollisionChannel::All);
	inputs = CreateComponent<InputComponent>();
	animation = CreateComponent<AnimationComponent>();
	camera = CreateComponent<CameraComponent>(/*FVector2(1920, 1080) / 2, FVector2(1920, 1080)*/);
}

void CustomActor::Construct()
{
	Actor::Construct();

	LOG_MSG(NAME_OF(CustomActor::Construct));

	sprite->useDebug = true;

	collision->BindCollisionResolution();

	inputs->Bind(&GetWorld()->GetInputManager()->D.onPerform, this, &CustomActor::MoveRight);
	inputs->Bind(&GetWorld()->GetInputManager()->Q.onPerform, this, &CustomActor::MoveLeft);
	inputs->Bind(&GetWorld()->GetInputManager()->D.onRelease, this, &CustomActor::Stop);
	inputs->Bind(&GetWorld()->GetInputManager()->Q.onRelease, this, &CustomActor::Stop);


	animation->AddAnimation("Idle", AnimationData(4, 0.5f, SpriteData(IVector2(0, 32 * 0), IVector2(32))));
	animation->AddAnimation("Move", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 1), IVector2(32))));
	animation->AddAnimation("Dash", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 3), IVector2(32))));
	animation->AddAnimation("Hit", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 4), IVector2(32)), false));
	animation->AddAnimation("Death", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 5), IVector2(32)), false));
	animation->AddAnimation("Sleep", AnimationData(4, 0.3f, SpriteData(IVector2(0, 32 * 6), IVector2(32))));

	camera->SetCurrent();
	//camera->attachedToOwner = true;
}

void CustomActor::BeginPlay()
{
	Actor::BeginPlay();

	LOG_MSG(NAME_OF(CustomActor::BeginPlay));

	animation->StartAnimation();
	LOG_MSG(GetWorld()->GetTimerManager()->CreateTimer(5.5f));
}

void CustomActor::Tick(const float& _deltaTime)
{
	Actor::Tick(_deltaTime);

	LOG_MSG(NAME_OF(CustomActor::Tick));
}

void CustomActor::Deconstruct()
{
	Actor::Deconstruct();

	LOG_MSG(NAME_OF(CustomActor::Deconstruct));
}

void CustomActor::BeginDestroy()
{
	Actor::BeginDestroy();

	LOG_MSG(NAME_OF(CustomActor::BeginDestroy));
}

void CustomActor::MoveLeft()
{
	if (!isRunning)
	{
		isRunning = true;
		animation->SetCurrentAnimation("Move");
		animation->StartAnimation();
	}
	transform.position += FVector2(-200.0f, 0) * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void CustomActor::MoveRight()
{
	if (!isRunning)
	{
		isRunning = true;
		animation->SetCurrentAnimation("Move");
		animation->StartAnimation();
	}
	transform.position += FVector2(200.0f, 0) * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void CustomActor::Stop()
{
	isRunning = false;
	animation->SetCurrentAnimation("Idle");
	animation->StartAnimation();
}
