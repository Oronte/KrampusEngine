#include "Background.h"

Background::Background(Level* _level)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(800, 300) * 4.0f, "Background"));
	animation = CreateComponent<AnimationComponent>();
}

void Background::Construct()
{
	Super::Construct();

	sprite->SetZOrder(ZOrder::Background);
	animation->AddAnimation("Base", AnimationData(8, 0.75f, IRect(IVector2::Zero(), IVector2(800, 300))));
}

void Background::BeginPlay()
{
	Super::BeginPlay();

	animation->StartAnimation();
}
