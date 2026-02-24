#include "CustomLevel.h"
#include "CustomActor.h"
#include "CustomWidget.h"
#include "Actors/SpriteActor.h"

void CustomLevel::InitLevel()
{
	const Krampus::FVector2& _windowCenter = GetWorld()->GetWindowRef().GetCenter();


	Krampus::Actor* _customActor = SpawnActor<CustomActor>();
	_customActor->transform.position = _windowCenter;
	_customActor->transform.rotation = Krampus::Angle(35.0f, true);

	Krampus::Widget* _customWidget = SpawnWidget<CustomWidget>(25.0f);
	_customWidget->SetScreenPosition(Krampus::FVector2(200.0f, 100.0f));

	Krampus::RectangleShapeData _data;
	_data.size = Krampus::FVector2(400.0f, 100.0f);
	_data.texturePath = "KrampusEngineBanner";			// Path from Content/Textures/
	Krampus::SpriteActor* _testCollision = SpawnActor<Krampus::SpriteActor>(_data);
	_testCollision->transform.position = _windowCenter;
}

void CustomLevel::Load()
{
	Krampus::Level::Load();


}

void CustomLevel::Unload()
{
	Krampus::Level::Unload();


}
