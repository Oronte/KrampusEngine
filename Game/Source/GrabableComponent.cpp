#include "GrabableComponent.h"
#include "Actors/Actor.h"
#include "Core/Engine.h"
#include "Graphics/Mouse.h"

GrabableComponent::GrabableComponent(Actor* _owner, Actor* _graber)
	: Component(_owner), graber(_graber)
{
}

void GrabableComponent::Tick(const float& _deltaTime)
{
	Super::Tick(_deltaTime);

	if (!graber) return;
	transform.position = graber->transform.position;
	transform.LookAt(GetWorld()->GetMouse()->GetPosition());
	if (FMath::Abs(transform.rotation.ToRadians()) >= FMath::halfPi)
		transform.scale = FVector2(1, -1);
	else transform.scale = FVector2::One();
}
