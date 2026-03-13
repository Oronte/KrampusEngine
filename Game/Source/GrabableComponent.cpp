#include "GrabableComponent.h"
#include "Actors/Actor.h"
#include "Core/Engine.h"
#include "Graphics/Mouse.h"

GrabableComponent::GrabableComponent(Actor* _owner, Player* _graber)
	: Component(_owner), graber(_graber)
{
}

void GrabableComponent::Construct()
{
	Super::Construct();

	handle = graber->onDeath.AddListener([this]()
		{
			canTrack = false;
		});
}

void GrabableComponent::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);

	if (!graber || !canTrack) return;
	transform.position = graber->transform.position;
	transform.LookAt(GetWorld()->GetMouse()->GetPosition());
	if (FMath::Abs(transform.rotation.ToRadians()) >= FMath::halfPi)
		transform.scale = FVector2(1, -1);
	else transform.scale = FVector2::One();
}
