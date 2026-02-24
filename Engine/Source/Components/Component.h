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
		class Actor* owner;
		class Level* level = nullptr;
		Transform& transform;

	public:
		inline Actor* GetOwner() const
		{
			return owner;
		}
		Engine* GetWorld() const;
		float GetDeltaTime() const;

	protected:
		Component(Actor* _owner);

		Timer* CreateTimer(const std::function<void()>& _callback, const float& _duration, const bool& _isLoop = false, const bool& _startRunning = true);

	public:
		virtual ~Component() = default;

	public:
		virtual void Construct() override;
		virtual void Deconstruct() override;
		virtual void BeginPlay() override {};
		virtual void Tick(const float& _deltaTime) override {};
		virtual void BeginDestroy() override {};

		virtual std::string ToString() const override;
	};


}

