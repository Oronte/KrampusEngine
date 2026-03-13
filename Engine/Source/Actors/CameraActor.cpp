#include "CameraActor.h"

Krampus::CameraActor::CameraActor(Level* _level)
	: Actor(_level)
{
	camera = CreateComponent<CameraComponent>();
	camera->SetCurrent();
}
