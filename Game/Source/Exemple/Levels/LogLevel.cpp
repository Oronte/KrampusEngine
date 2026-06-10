#include "LogLevel.h"
#include "Exemple/UI/MainMenuButton.h"
#include "Exemple/Levels/MainMenu.h"

Krampus::Exemple::LogLevel::LogLevel(Engine* _world)
	: Level(_world)
{
	backgroundColor = Color::Yellow();
}

void Krampus::Exemple::LogLevel::InitLevel()
{
	Super::InitLevel();

	RectangleShapeData _dataBack;
	_dataBack.size = FVector2(200.0f, 100.0f);
	MainMenuButton* _backButton = SpawnWidget<MainMenuButton>(_dataBack, "Back");
	_backButton->SetScreenPosition(_dataBack.size / 2.0f);
	_backButton->BindChangeLevel<MainMenu>();

	CircleShapeData _data;
	_data.radius = 125.0f;
	MainMenuButton* _messageLogButton = SpawnWidget<MainMenuButton>(_data, "Message Log");
	MainMenuButton* _warningLogButton = SpawnWidget<MainMenuButton>(_data, "Warning Log");
	MainMenuButton* _errorLogButton = SpawnWidget<MainMenuButton>(_data, "Error Log");
	MainMenuButton* _fatalLogButton = SpawnWidget<MainMenuButton>(_data, "Fatal Log\n(Crash the game)");

	const FVector2 _center = FVector2(1920.0f, 1080.0f) / 2.0f;
	const FVector2 _offset = FVector2(1920.0f, 1080.0f) / 8.0f;

	_messageLogButton->SetScreenPosition(_center - _offset);
	_warningLogButton->SetScreenPosition(_center + FVector2(_offset.x, -_offset.y));
	_errorLogButton->SetScreenPosition(_center + FVector2(-_offset.x, _offset.y));
	_fatalLogButton->SetScreenPosition(_center + _offset);

	messageLogHandle = _messageLogButton->onPress.AddListener([this]()
		{
			LOG_MSG("This is a Message Log");
		});
	warningLogHandle = _warningLogButton->onPress.AddListener([this]()
		{
			LOG_WARNING("This is a Warning Log");
		});
	errorLogHandle = _errorLogButton->onPress.AddListener([this]()
		{
			LOG_ERROR("This is a Error Log");
		});
	fatalLogHandle = _fatalLogButton->onPress.AddListener([this]()
		{
			LOG_FATAL("This is a Fata Log");
		});
}
