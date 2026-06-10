#include "AnimationExemple.h"
#include "Components/SpriteComponent.h"

Krampus::Exemple::AnimationExemple::AnimationExemple(Level* _level)
	: Actor(_level)
{
	RectangleShapeData _data;
	_data.size = FVector2(250.0f);
	_data.texturePath = "Explosion";
	CreateComponent<SpriteComponent>(_data);
	animation = CreateComponent<AnimationComponent>();
}

void Krampus::Exemple::AnimationExemple::Construct()
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

	animation->AddAnimation("Exemple", AnimationData(1.0f, _frames/*, canLoop=true*/));
}

void Krampus::Exemple::AnimationExemple::BeginPlay()
{
	Super::BeginPlay();

	animation->SetCurrentAnimation("Exemple");
	animation->StartAnimation();
}

void Krampus::Exemple::AnimationExemple::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);
}

void Krampus::Exemple::AnimationExemple::Deconstruct()
{
	Super::Deconstruct();
}

void Krampus::Exemple::AnimationExemple::BeginDestroy()
{
	Super::BeginDestroy();
}
