#pragma once
#include "Core/GameObject.h"
#include "Utilities/Math/Transform.h"
#include "Utilities/System/Timer.h"

namespace Krampus
{
	class Engine;

	class Component : public IGameObject
	{
	protected:
		class Actor*	owner;
		class Level*	level		= nullptr;
		Transform&		transform;

	public:
		inline Actor* GetOwner() const
		{
			return owner;
		}
		Engine* GetWorld() const;
		Float GetDeltaTime() const;

	protected:
		Component(Actor* _owner);

		Timer* CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);

	public:
		virtual ~Component() = default;

	public:
		virtual void Construct() override;
		virtual void Deconstruct() override;
		virtual void BeginPlay() override {};
		virtual void Tick(const Float& _deltaTime) override {};
		virtual void BeginDestroy() override {};

		virtual std::string ToString() const override;
	};


}

