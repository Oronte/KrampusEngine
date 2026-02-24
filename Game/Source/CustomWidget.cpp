#include "CustomWidget.h"


CustomWidget::CustomWidget(Krampus::Level* _level, float _startTime)
	: Widget(_level)
{
	time = _startTime;
	label = CreateComponent<Krampus::LabelComponent>(Krampus::TextData(std::format("Exemple Widget -> The app has been launch for {} seconds", time)));
}

void CustomWidget::Construct()
{
	Super::Construct();
}

void CustomWidget::BeginPlay()
{
	Super::BeginPlay();
}

void CustomWidget::Tick(const float& _deltaTime)
{
	Super::Tick(_deltaTime);

	time += GetDeltaTime();
	UpdateText();
}

void CustomWidget::Deconstruct()
{
	Super::Deconstruct();
}

void CustomWidget::BeginDestroy()
{
	Super::BeginDestroy();
}

void CustomWidget::UpdateText()
{
	label->SetString(std::format("Exemple Widget -> The app has been launch for {} seconds", time));
}
