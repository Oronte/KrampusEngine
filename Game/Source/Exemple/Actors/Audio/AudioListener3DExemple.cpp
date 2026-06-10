#include "AudioListener3DExemple.h"
#include "Components/SpriteComponent.h"
#include "Graphics/Mouse.h"

Krampus::Exemple::AudioListener3DExemple::AudioListener3DExemple(Level* _level)
	: Actor(_level)
{
	listener = CreateComponent<AudioListenerComponent>(true);
	RectangleShapeData _data;
	_data.size = FVector2(344.0f, 538.0f) / 3.5f;
	_data.texturePath = "Ear";
	CreateComponent<SpriteComponent>(_data)->SetZOrder(ZOrder::Debug);
}

void Krampus::Exemple::AudioListener3DExemple::Construct()
{
	Super::Construct();
}

void Krampus::Exemple::AudioListener3DExemple::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetMouse()->Hide();
}

void Krampus::Exemple::AudioListener3DExemple::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);

	SetActorPosition(GetWorld()->GetMouse()->GetPosition());
}

void Krampus::Exemple::AudioListener3DExemple::Deconstruct()
{
	Super::Deconstruct();
}

void Krampus::Exemple::AudioListener3DExemple::BeginDestroy()
{
	Super::BeginDestroy();

	GetWorld()->GetMouse()->Show();
}
