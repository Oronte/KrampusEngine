#include "TransitionLevel.h"
#include "Actors/SpriteActor.h"

void Krampus::TransitionLevel::InitLevel()
{
	RectangleShapeData _data;
	_data.size = FVector2(1920.0f, 1080.0f);
	_data.texturePath = "KrampusBanner";
	SpawnActor<SpriteActor>(_data);
}
