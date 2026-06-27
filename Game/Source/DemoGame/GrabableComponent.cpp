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
	SetActorPosition(graber->GetActorPosition());
	LookAt(GetWorld()->GetMouse()->GetPosition());
	if (FMath::Abs(GetActorRotation().ToRadians()) >= FMath::halfPi)
		SetActorScale(FVector2(1, -1));
	else SetActorScale(FVector2::One());
}
