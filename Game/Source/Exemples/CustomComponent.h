#pragma once
#include "Components/Component.h"

class CustomComponent : public Krampus::Component
{
	float moveSpeed = 100.0f;

	std::vector<Krampus::Event<>::ListenerHandle> eventHandles;
	Krampus::Event<Krampus::IVector2>::ListenerHandle mouseMovedEventHandle;

public:
	CustomComponent(Krampus::Actor* _owner, float _moveSpeed);

	virtual void BeginPlay() override;

	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();

	void Rotate(const Krampus::IVector2& _mousePos);
};

