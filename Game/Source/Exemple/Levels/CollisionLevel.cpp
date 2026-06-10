#include "CollisionLevel.h"
#include "MainMenu.h"
#include "Exemple/UI/MainMenuButton.h"
#include "Exemple/Actors/Collision/CollisionActorExemple.h"
#include "Exemple/Actors/Collision/RigidbodyExemple.h"

Krampus::Exemple::CollisionLevel::CollisionLevel(Engine* _world)
	: Level(_world)
{
	backgroundColor = Color(239, 68, 68);
}

void Krampus::Exemple::CollisionLevel::InitLevel()
{
	Super::InitLevel();

	RectangleShapeData _data;
	_data.size = FVector2(200.0f, 100.0f);
	MainMenuButton* _backButton = SpawnWidget<MainMenuButton>(_data, "Back");
	_backButton->SetScreenPosition(_data.size * FVector2(3.5f, 1.0f));
	_backButton->BindChangeLevel<MainMenu>();

	RectangleShapeData _data2;
	_data2.size = FVector2(200.0f, 100.0f);
	MainMenuButton* _resetButton = SpawnWidget<MainMenuButton>(_data2, "Reset Simulation");
	_resetButton->SetScreenPosition(_data.size * FVector2(5.5f, 1.0f));
	onResetHandle = _resetButton->onPress.AddListener(this, &CollisionLevel::ResetSimulation);

	SpawnActorAt<CollisionActorExemple>(Transform(FVector2(1920.0f / 2.0f, 1000.0f)));
	SpawnActorAt<CollisionActorExemple>(Transform(FVector2(100.0f, 500.0f), Angle(FMath::halfPi)));
	SpawnActorAt<CollisionActorExemple>(Transform(FVector2(1850.0f, 0.0f), Angle(FMath::halfPi)));

	ResetSimulation();
}

void Krampus::Exemple::CollisionLevel::ResetSimulation()
{
	for (Actor* _actor : rbs)
		_actor->Destroy();
	rbs.clear();

	FVector2 _start = FVector2(400.0f, 100.0f);

	for (int _indexX = 0; _indexX < 5; _indexX++)
		for (int _indexY = 0; _indexY < 2; _indexY++)
			rbs.push_back(SpawnActorAt<RigidbodyExemple>(Transform(_start + FVector2(300.0f * _indexX, 200.0f * _indexY), Angle(FMath::RandomRange(0.0f, 180.0f)))));
}
