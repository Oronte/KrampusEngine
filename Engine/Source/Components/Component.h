#pragma once
#include "Core/GameObject.h"
#include "Utilities/System/Transformable.h"
#include "Utilities/System/Timer.h"

namespace Krampus
{
	class Engine;

	class Component : public IGameObject, public ITransformable
	{
	protected:
		class Actor*	owner;

	public:
		inline Actor* GetOwner() const
		{
			return owner;
		}
		class Level* GetLevel() const;
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

		void SetActorPosition(const FVector2& _newPosition) override;
		FVector2 GetActorPosition() const override;
		void SetActorRotation(const Angle& _newRotation) override;
		Angle GetActorRotation() const override;
		void SetActorScale(const FVector2& _newScale) override;
		FVector2 GetActorScale() const override;
	};


}

