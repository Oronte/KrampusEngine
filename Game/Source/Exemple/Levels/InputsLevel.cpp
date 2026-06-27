#include "InputsLevel.h"
#include "Exemple/Levels/MainMenu.h"
#include "Exemple/UI/MainMenuButton.h"
#include "Exemple/Actors/Inputs/InputActorExemple.h"
#include "UI/LabelWidget.h"

Krampus::Exemple::InputsLevel::InputsLevel(Engine* _world)
	: Level(_world)
{
}

void Krampus::Exemple::InputsLevel::InitLevel()
{
	Super::InitLevel();

	RectangleShapeData _data;
	_data.size = FVector2(200.0f, 100.0f);
	MainMenuButton* _backButton = SpawnWidget<MainMenuButton>(_data, "Back");
	_backButton->SetScreenPosition(_data.size / 2.0f);
	_backButton->BindChangeLevel<MainMenu>();

	SpawnActor<InputActorExemple>();
	LabelWidget* _title = SpawnWidget<LabelWidget>(TextData("Press any input and see result at the Console"));
	_title->SetScreenPosition(FVector2(1920.0f/2.0f, 350.0f));
	_title->SetFillColor(Color::Red());

	LabelWidget* _content = SpawnWidget<LabelWidget>(TextData("Inputs have 3 Events :\n- onPress (Trigger one time when the key is Pressed)\n- onPerform (Trigger every frames while the key is Pressed)\n- onRelease (Trigger one time when the input is Released)"));
	_content->SetScreenPosition(FVector2(1920.0f / 2.0f, 550.0f));
	_title->SetFillColor(Color::Yellow());

}
