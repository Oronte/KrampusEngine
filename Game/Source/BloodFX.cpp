#include "BloodFX.h"
#include "Managers/TimerManager.h"

BloodFX::BloodFX(Level* _level)
	: Actor(_level)
{
	transform.scale = FVector2(-1, 1);
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(110, 93) * 2.0f, "FX/BloodFX"));
	animation = CreateComponent<AnimationComponent>();
	animation->AddAnimation("Hit", AnimationData(10, 0.75f, SpriteData(IVector2(110, 93), IVector2(110, 93)), false));
	handle = animation->GetCurrentAnimation()->notifies[9].AddListener([this] {Destroy(); });
	animation->StartAnimation();

	//handle = GetWorld()->GetTimerManager()->CreateTimer(1.0f)->callback.AddListener([this] {Destroy(); });
}
