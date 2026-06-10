#include "AudioLevel.h"
#include "Exemple/Actors/Audio/AudioListener3DExemple.h"
#include "Exemple/Actors/Audio/AudioExemple.h"
#include <Exemple/UI/MainMenuButton.h>
#include "MainMenu.h"
#include <UI/LabelWidget.h>

Krampus::Exemple::AudioLevel::AudioLevel(Engine* _world)
	: Level(_world)
{
	backgroundColor = Color(34, 197, 94);
}

void Krampus::Exemple::AudioLevel::InitLevel()
{
	Super::InitLevel();
	RectangleShapeData _data;
	_data.size = FVector2(200.0f, 100.0f);
	MainMenuButton* _backButton = SpawnWidget<MainMenuButton>(_data, "Back");
	_backButton->SetScreenPosition(_data.size / 2.0f);
	_backButton->BindChangeLevel<MainMenu>();

	SpawnActor<AudioListener3DExemple>();
	SpawnActorAt<AudioExemple>(Transform(FVector2(1920.0f, 1080.0f)/2.0f, Angle(0.0f)));

	LabelWidget* _spatialSoundTitle = SpawnWidget<LabelWidget>(TextData("SpatialSoundComponent + AudioListenerComponent", 45U));
	_spatialSoundTitle->SetFillColor(Color::Red());
	_spatialSoundTitle->SetScreenPosition(FVector2(950.0f, 75.0f));

	LabelWidget* _spatialSoundContent = SpawnWidget<LabelWidget>(TextData(
		"SpatialSoundComponent\n- Positions a sound in 3D space based on the actor s movements and rotations\n- Supports two modes: top-down and side view, which changes the direction vector used\n- Automatically updates the sound velocity each Tick to simulate the Doppler effect\n- Exposes the full Sound API (volume, pitch, pan, cone, min/max gain, playing offset...) via inline wrappers\n\nAudioListenerComponent\n- Represents the player s ears\n- Also supports two modes (topView / side) with different up and forward vectors", 20U));
	_spatialSoundContent->SetFillColor(Color::Blue());
	_spatialSoundContent->SetScreenPosition(FVector2(900.0f, 250.0f));

	LabelWidget* _spatialSoundTip = SpawnWidget<LabelWidget>(TextData(
		"(You can move mouve around the sound with your mouse)", 20U));
	_spatialSoundTip->SetFillColor(Color::Yellow());
	_spatialSoundTip->SetScreenPosition(FVector2(1500.0f, 500.0f));
}
