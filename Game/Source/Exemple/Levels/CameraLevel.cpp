#include "CameraLevel.h"
#include "Exemple/UI/MainMenuButton.h"
#include "UI/LabelWidget.h"
#include "Exemple/Levels/MainMenu.h"
#include "Exemple/Actors/Camera/CameraActorExemple.h"
#include "Exemple/Actors/Camera/SpriteActorExemple.h"

Krampus::Exemple::CameraLevel::CameraLevel(Engine* _world)
	: Level(_world)
{
	backgroundColor = Color(59, 130, 246);
}

void Krampus::Exemple::CameraLevel::InitLevel()
{
	Super::InitLevel();

	RectangleShapeData _rectData;
	_rectData.size = FVector2(200.0f, 100.0f);
	MainMenuButton* _backButton = SpawnWidget<MainMenuButton>(_rectData, "Back");
	_backButton->SetScreenPosition(_rectData.size / 2.0f);
	_backButton->BindChangeLevel<MainMenu>();


	SpawnActorAt<CameraActorExemple>(Transform(FVector2(1920.0f, 1080.0f) / 2.0f));

	RectangleShapeData _data;
	_data.size = FVector2(1024.0f, 559.0f);
	_data.texturePath = "_internal/KrampusEngineBanner";
	SpawnActorAt<SpriteActorExemple>(Transform(FVector2(1920.0f, 1080.0f) / 2.0f), _data);
	
	CircleShapeData _circleData;
	_circleData.texturePath = "_internal/KrampusLogo";
	SpawnActorAt<SpriteActorExemple>(Transform(FVector2(350.0f)), _circleData);

	LabelWidget* _tip = SpawnWidget<LabelWidget>(TextData(
		"Move Mouse -> Move Camera\nScroll Mouse Wheel -> Rotate Camera"));
	_tip->SetFillColor(Color::Yellow());
	_tip->SetScreenPosition(FVector2(1300.0f, 700.0f));
}
