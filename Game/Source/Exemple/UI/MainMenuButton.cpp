#include "MainMenuButton.h"
#include "Components/LabelComponent.h"

Krampus::Exemple::MainMenuButton::MainMenuButton(Level* _level, const CircleShapeData& _data, const String& _text)
	: ButtonWidget(_level, _data)
{
	TextData _textData;
	_textData.text = _text;
	_textData.characterSize = 25;
	LabelComponent* _label = CreateComponent<LabelComponent>(_textData);
	_label->SetZOrder(ZOrder::Debug);
}

Krampus::Exemple::MainMenuButton::MainMenuButton(Level* _level, const RectangleShapeData& _data, const String& _text)
	: ButtonWidget(_level, _data)
{
	TextData _textData;
	_textData.text = _text;
	_textData.characterSize = 25;
	LabelComponent* _label = CreateComponent<LabelComponent>(_textData);
	_label->SetZOrder(ZOrder::Debug);
}

void Krampus::Exemple::MainMenuButton::Construct()
{
	Super::Construct();

	onHoverHandle = onHover.AddListener([this]()
		{
			SetActorScale(hoverScale);
		});
	onUnhoverHandle = onUnhover.AddListener([this]()
		{
			SetActorScale(unhoverScale);
		});
}

void Krampus::Exemple::MainMenuButton::BeginPlay()
{
	Super::BeginPlay();
}

void Krampus::Exemple::MainMenuButton::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);
}

void Krampus::Exemple::MainMenuButton::Deconstruct()
{
	Super::Deconstruct();
}

void Krampus::Exemple::MainMenuButton::BeginDestroy()
{
	Super::BeginDestroy();
}
