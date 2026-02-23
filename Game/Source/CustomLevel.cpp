#include "CustomLevel.h"
#include "CustomActor.h"
#include "CustomWidget.h"

void CustomLevel::InitLevel()
{
	Krampus::Actor* _actor = SpawnActor<CustomActor>();
	_actor->transform.position = Krampus::FVector2(300, 300);
}

void CustomLevel::Load()
{
	Krampus::Level::Load();


}

void CustomLevel::Unload()
{
	Krampus::Level::Unload();


}
