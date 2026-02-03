#include "CustomComponent.h"
#include "Core/Engine.h"
#include "Managers/TimerManager.h"
#include "Managers/InputManager.h"

CustomComponent::CustomComponent(Krampus::Actor* _owner, float _moveSpeed)
	: Component(_owner)
{
	moveSpeed = _moveSpeed;
}

void CustomComponent::BeginPlay()
{
	Component::BeginPlay();

	mouseMovedEventHandle = GetWorld()->GetInputManager()->MouseMovedWorld.AddListener(this, &CustomComponent::Rotate);
	eventHandles.push_back(GetWorld()->GetInputManager()->Z.onPerform.AddListener(this, &CustomComponent::MoveForward));
	eventHandles.push_back(GetWorld()->GetInputManager()->S.onPerform.AddListener(this, &CustomComponent::MoveBackward));
	eventHandles.push_back(GetWorld()->GetInputManager()->D.onPerform.AddListener(this, &CustomComponent::MoveRight));
	eventHandles.push_back(GetWorld()->GetInputManager()->Q.onPerform.AddListener(this, &CustomComponent::MoveLeft));
}

void CustomComponent::MoveForward()
{
	transform.position += Krampus::FVector2::Right() * moveSpeed * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void CustomComponent::MoveBackward()
{
	transform.position += Krampus::FVector2::Left() * moveSpeed * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void CustomComponent::MoveRight()
{
	transform.position += Krampus::FVector2::Down() * moveSpeed * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void CustomComponent::MoveLeft()
{
	transform.position += Krampus::FVector2::Up() * moveSpeed * GetWorld()->GetTimerManager()->GetDeltaTime();
}

void CustomComponent::Rotate(const Krampus::IVector2& _mousePos)
{
	transform.LookAt(_mousePos);
	LOG_MSG(_mousePos);
}
