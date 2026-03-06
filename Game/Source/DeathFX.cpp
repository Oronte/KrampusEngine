#include "DeathFX.h"

DeathFX::DeathFX(Level* _level)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(200.0f), "Explosion"));
	animation = CreateComponent<AnimationComponent>();
}

void DeathFX::Construct()
{
	Super::Construct();

	IVector2 _spriteData = IVector2(100);
	std::vector<IRect> _frames;

	for (int _i = 0; _i < 7; _i++)
	{
		for (int _y = 0; _y < 10; _y++)
		{
			_frames.push_back(IRect(IVector2(100 * _y, 100 * _i), _spriteData));
		}
	}

	animation->AddAnimation("explo", AnimationData(1.0f, _frames, false));
	handle = animation->GetCurrentAnimation()->notifies[CAST(int, _frames.size()) - 1].AddListener([this]()
		{
			Destroy();
		});
}

void DeathFX::BeginPlay()
{
	Super::BeginPlay();

	animation->StartAnimation();
}
