#pragma once
#include "CustomComponent.h"
#include "Actors/SpriteActor.h"
#include "Components/CameraComponent.h"

class CustomActor : public Krampus::SpriteActor
{
	CustomComponent* movement = nullptr;
	Krampus::CameraComponent* camera = nullptr;

public:
	CustomActor(Krampus::Level* _level, const Krampus::RectangleShapeData& _data);
};

