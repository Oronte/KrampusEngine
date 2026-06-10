#include "AnimationLevel.h"
#include "UI/LabelWidget.h"
#include "Exemple/Actors/Animation/AnimationExemple.h"
#include "Exemple/Actors/Animation/AnimationSMExemple.h"
#include "Exemple/UI/MainMenuButton.h"
#include "MainMenu.h"

Krampus::Exemple::AnimationLevel::AnimationLevel(Engine* _world)
	: Level(_world)
{
	backgroundColor = Color(139, 92, 246);
}

void Krampus::Exemple::AnimationLevel::InitLevel()
{
	Super::InitLevel();
	
	RectangleShapeData _data;
	_data.size = FVector2(200.0f, 100.0f);
	MainMenuButton* _backButton = SpawnWidget<MainMenuButton>(_data, "Back");
	_backButton->SetScreenPosition(_data.size / 2.0f);
	_backButton->BindChangeLevel<MainMenu>();


	// =========	AnimationComponent	=========
	SpawnActorAt<AnimationExemple>(Transform(
	FVector2(400.0f, 400.0f), Angle(0.0f)));

	LabelWidget* _animationComponentTitle = SpawnWidget<LabelWidget>(TextData("AnimationComponent", 45U));
	_animationComponentTitle->SetFillColor(Color::Red());
	_animationComponentTitle->SetScreenPosition(FVector2(400.0f, 550.0f));

	LabelWidget* _animationComponentContent = SpawnWidget<LabelWidget>(TextData(
		"- Manages an actor s animations.\n- It stores multiple animations.\n- It selects and switches the current animation.\n- It controls playback: start, stop, pause, resume.\n- (It requires a SpriteComponent to apply animations.)"));
	_animationComponentContent->SetFillColor(Color::Yellow());
	_animationComponentContent->SetScreenPosition(FVector2(400.0f, 700.0f));


	// =========	AnimationSMComponent	=========
	SpawnActorAt<AnimationSMExemple>(Transform(
	FVector2(1150.0f, 400.0f), Angle(0.0f)));

	LabelWidget* _animationComponentSMTitle = SpawnWidget<LabelWidget>(TextData("AnimationComponent (StateMachine)", 45U));
	_animationComponentSMTitle->SetFillColor(Color::Red());
	_animationComponentSMTitle->SetScreenPosition(FVector2(1200.0f, 550.0f));

	LabelWidget* _animationComponentSMContent = SpawnWidget<LabelWidget>(TextData(
		"- Manages an actor s animation state machine.\n- It stores multiple animations and their transition links.\n- It automatically switches animations based on transition conditions.\n- It controls playback : start, stop, pause, resume.\n- It allows recursive animation state transitions through conditional links.\n- (It requires a SpriteComponent to apply animations.)"));
	_animationComponentSMContent->SetFillColor(Color::Yellow());
	_animationComponentSMContent->SetScreenPosition(FVector2(1300.0f, 700.0f));

}
