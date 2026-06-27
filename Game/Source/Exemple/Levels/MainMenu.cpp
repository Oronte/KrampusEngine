#include "MainMenu.h"
#include "Managers/LevelManager.h"
#include "Exemple/UI/MainMenuButton.h"
#include "UI/ImageWidget.h"
#include "AnimationLevel.h"
#include "AudioLevel.h"
#include "CameraLevel.h"
#include "CollisionLevel.h"
#include "InputsLevel.h"
#include "LogLevel.h"

#include "DemoGame/MainMenuDemoGame.h"

Krampus::Exemple::MainMenu::MainMenu(Engine* _world)
	: Level(_world)
{

}

void Krampus::Exemple::MainMenu::InitLevel()
{
	Super::InitLevel();

	RectangleShapeData _backgroundData;
	_backgroundData.size = FVector2(1920.0f, 1080.0f);
	_backgroundData.texturePath = "_internal/KrampusEngineBanner";
	ImageWidget* _background =
		SpawnWidget<ImageWidget>(_backgroundData);
	_background->SetScreenPosition(FVector2(1920.0f, 1080.0f) / 2.0f);

	const FVector2& _buttonSize = FVector2(200.0f, 100.0f);
	const FVector2& _spacer = FVector2(_buttonSize.x * 1.5f, 0.0f);

	RectangleShapeData _quit;
	_quit.size = _buttonSize;
	MainMenuButton* _quitButton =
		SpawnWidget<MainMenuButton>(_quit, "Quit");
	_quitButton->SetScreenPosition(FVector2(1920.0f / 2.0f, 950.0f));
	quitHandle = _quitButton->onPress.AddListener([this]()
		{
			GetWorld()->QuitGame();
		});


	// ==============	Animation	==============
	RectangleShapeData _animation;
	_animation.size = _buttonSize;
	MainMenuButton* _animationButton =
		SpawnWidget<MainMenuButton>(_animation, "Animation");
	_animationButton->SetScreenPosition(_buttonSize);
	_animationButton->BindChangeLevel<AnimationLevel>();


	// ==============	Audio	==============
	RectangleShapeData _audio;
	_audio.size = _buttonSize;
	MainMenuButton* _audioButton =
		SpawnWidget<MainMenuButton>(_audio, "Audio 3D");
	_audioButton->SetScreenPosition(_buttonSize + _spacer);
	_audioButton->BindChangeLevel<AudioLevel>();


	// ==============	Camera	==============
	RectangleShapeData _camera;
	_camera.size = _buttonSize;
	MainMenuButton* _cameraButton =
		SpawnWidget<MainMenuButton>(_camera, "Camera");
	_cameraButton->SetScreenPosition(_buttonSize + _spacer * 2);
	_cameraButton->BindChangeLevel<CameraLevel>();


	// ==============	Physics		==============
	RectangleShapeData _collision;
	_collision.size = _buttonSize;
	MainMenuButton* _collisionButton =
		SpawnWidget<MainMenuButton>(_collision, "Physics");
	_collisionButton->SetScreenPosition(_buttonSize + _spacer * 3);
	_collisionButton->BindChangeLevel<CollisionLevel>();


	// ==============	LOG		==============
	RectangleShapeData _log;
	_log.size = _buttonSize;
	MainMenuButton* _logButton =
		SpawnWidget<MainMenuButton>(_log, "Logs");
	_logButton->SetScreenPosition(_buttonSize + _spacer * 4);
	_logButton->BindChangeLevel<LogLevel>();


	// ==============	Input	==============
	RectangleShapeData _inputs;
	_inputs.size = _buttonSize;
	MainMenuButton* _inputButton =
		SpawnWidget<MainMenuButton>(_inputs, "Inputs");
	_inputButton->SetScreenPosition(_buttonSize + _spacer * 5);
	_inputButton->BindChangeLevel<InputsLevel>();





	// ==============	Test Game	==============
	RectangleShapeData _testGame;
	_testGame.size = _buttonSize;
	MainMenuButton* _testGameButton =
		SpawnWidget<MainMenuButton>(_testGame, "Test Game");
	_testGameButton->SetScreenPosition(FVector2(1920.0f, 1080.0f) - _buttonSize * 2.0f);
	_testGameButton->BindChangeLevel<MainMenuDemoGame>();
}
