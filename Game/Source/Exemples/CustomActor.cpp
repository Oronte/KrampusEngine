#include "CustomActor.h"

CustomActor::CustomActor(Krampus::Level* _level, const Krampus::RectangleShapeData& _data)
	: SpriteActor(_level, _data)
{
	movement = CreateComponent<CustomComponent>(200.0f);
	camera = CreateComponent<Krampus::CameraComponent>();
	camera->attachedToOwner = true;
	camera->freezeRotation = true;
}
