#include "MovementComponent.h"

namespace Krampus
{

MovementComponent::MovementComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(MovementComponent);
}

void MovementComponent::MoveLeft()
{
	Move(FVector2(-moveSpeed, 0.0f));
}

void MovementComponent::MoveRight()
{
	Move(FVector2(moveSpeed, 0.0f));
}


}
