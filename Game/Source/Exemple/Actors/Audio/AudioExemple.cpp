#include "AudioExemple.h"
#include "Managers/AudioManager.h"
#include <Components/SpriteComponent.h>

Krampus::Exemple::AudioExemple::AudioExemple(Level* _level)
	: Actor(_level)
{
	RectangleShapeData _data;
	_data.size = FVector2(150.0f);
	_data.texturePath = "Audio";
	CreateComponent<SpriteComponent>(_data);
	soundComponent = CreateComponent<SpatialSoundComponent>("MainMenuMusic", AudioExtensionType::MP3);
}

void Krampus::Exemple::AudioExemple::Construct()
{
	Super::Construct();

	soundComponent->playOnBeginPlay = true;
	soundComponent->SetLoop(true);
}

void Krampus::Exemple::AudioExemple::BeginPlay()
{
	Super::BeginPlay();

}

void Krampus::Exemple::AudioExemple::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);

}

void Krampus::Exemple::AudioExemple::Deconstruct()
{
	Super::Deconstruct();
}

void Krampus::Exemple::AudioExemple::BeginDestroy()
{
	Super::BeginDestroy();
}
