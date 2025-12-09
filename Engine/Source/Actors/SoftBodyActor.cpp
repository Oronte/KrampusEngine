#include "SoftBodyActor.h"
#include "GameFramework/Level.h"

using namespace Krampus;

SoftBodyActor::SoftBodyActor(Level* _level, const FVector2& _size, int gridW, int gridH, float kstruct, float kshear)
	: Actor(_level)
{
	softBody = CreateComponent<SoftBodyComponent>(gridW, gridH, _size, kstruct, kshear);
}

SoftBodyActor::SoftBodyActor(Level* _level, const float& _radius, int pointCount, float kstruct, float kshear, float kradial)
	: Actor(_level)
{
	softBody = CreateComponent<SoftBodyComponent>(pointCount, _radius, kstruct, kshear, kradial);
}