#include "AnimationSMExemple.h"
#include "Components/SpriteComponent.h"

Krampus::Exemple::AnimationSMExemple::AnimationSMExemple(Level* _level)
	: Actor(_level)
{
	CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(250.0f), "Player"));
	animation = CreateComponent<AnimationComponentSM>();
}

void Krampus::Exemple::AnimationSMExemple::Construct()
{
	Super::Construct();

	AnimationSM* _idle = animation->AddAnimation("Idle", AnimationData(4, 0.6f, IRect(IVector2(0, 32 * 0), IVector2(32))));
	AnimationSM* _move = animation->AddAnimation("Move", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 1), IVector2(32))));
	AnimationSM* _jump = animation->AddAnimation("Jump", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 2), IVector2(32))));
	AnimationSM* _hit = animation->AddAnimation("Hit", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 4), IVector2(32)), false));

	_idle->CreateLink(_move, [this]() -> bool
		{
			if (canUpdateAnimation)
			{
				canUpdateAnimation = false;
				return true;
			}
			return false;
		});

	_move->CreateLink(_jump, [this]() -> bool
		{
			if (canUpdateAnimation)
			{
				canUpdateAnimation = false;
				return true;
			}
			return false;
		});

	_jump->CreateLink(_hit, [this]() -> bool
		{
			if (canUpdateAnimation)
			{
				canUpdateAnimation = false;
				return true;
			}
			return false;
		});

	_hit->CreateLink(_idle, [this]() -> bool
		{
			if (canUpdateAnimation)
			{
				canUpdateAnimation = false;
				return true;
			}
			return false;
		});
}

void Krampus::Exemple::AnimationSMExemple::BeginPlay()
{
	Super::BeginPlay();

	animation->StartAnimation();
}

void Krampus::Exemple::AnimationSMExemple::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);

	time += _deltaTime;
	if (time >= maxTime)
	{
		time = 0.0f;
		canUpdateAnimation = true;
	}
}

void Krampus::Exemple::AnimationSMExemple::Deconstruct()
{
	Super::Deconstruct();
}

void Krampus::Exemple::AnimationSMExemple::BeginDestroy()
{
	Super::BeginDestroy();
}
