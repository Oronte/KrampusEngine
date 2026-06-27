#include "MainMenuDemoGame.h"
#include "UI/ImageWidget.h"
#include "NameButtonWidget.h"
#include "Managers/LevelManager.h"
#include "DemoLevel.h"
#include "Managers/AudioManager.h"
#include "Exemple/Levels/MainMenu.h"

MainMenuDemoGame::MainMenuDemoGame(Engine* _world)
	: Level(_world)
{
}

void MainMenuDemoGame::InitLevel()
{
	Super::InitLevel();

	ImageWidget* _background = SpawnWidget<ImageWidget>(RectangleShapeData(FVector2(1920, 1080), "_internal/KrampusEngineBanner"));
	_background->SetScreenPosition(FVector2(1920, 1080) / 2.0f);

	NameButtonWidget* _playButton = SpawnWidget<NameButtonWidget>(RectangleShapeData(FVector2(400, 150)), "Play");
	_playButton->SetScreenPosition(FVector2(1920, 1080) / 2.0f);
	onClickPlayHandle = _playButton->onPress.AddListener([this]()
		{
			GetWorld()->GetLevelManager()->SetLevel<DemoLevel>();
		});

	NameButtonWidget* _quitButton = SpawnWidget<NameButtonWidget>(RectangleShapeData(FVector2(400, 150)), "Quit");
	_quitButton->SetScreenPosition((FVector2(1920, 1080) / 2.0f) + FVector2::Down() * 300.0f);
	onClickQuitHandle = _quitButton->onPress.AddListener([this]()
		{
			GetWorld()->GetLevelManager()->SetLevel<Krampus::Exemple::MainMenu>();
		});

	music = M_AUDIO.CreateMusic("MainMenuMusic", AudioExtensionType::MP3);
	music.SetLoop(true);
	music.Play();
}

void MainMenuDemoGame::Unload()
{
	Super::Unload();
	music.Stop();
}
