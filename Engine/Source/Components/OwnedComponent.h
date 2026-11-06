#pragma once
#include "Core/GameObject.h"
#include "Utilities/System/Transformable.h"
#include "Utilities/System/Timer.h"

namespace Krampus
{
	class Engine;
	class Level;

	/// <summary>
	/// A part of an owner (an Actor or a Widget): it lives in the owner's level and shares the owner's transform.
	/// Base of Component and WidgetComponent; explicitly instantiated for both owners in OwnedComponent.cpp.
	/// </summary>
	template <typename Owner>
	class OwnedComponent : public IGameObject, public ITransformable
	{
	public:
		using OwnerType = Owner;

	protected:
		Owner* owner;

		explicit OwnedComponent(Owner* _owner) : owner(_owner) {}

		/// <summary>Creates a timer ticked by the owner's level.</summary>
		Timer* CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);

	public:
		[[nodiscard]] Owner* GetOwner() const { return owner; }
		[[nodiscard]] Level* GetLevel() const;
		[[nodiscard]] Engine* GetWorld() const;
		[[nodiscard]] Float GetDeltaTime() const;

		void Construct() override	{ SetActive(true); }
		void Deconstruct() override	{ SetActive(false); }
		void BeginPlay() override {}
		void Tick(const Float&) override {}
		void BeginDestroy() override {}
		std::string ToString() const override;

		/// The transform is the owner's: setting it moves the owner.
		void SetPosition(const FVector2& _position) override;
		FVector2 GetPosition() const override;
		void SetRotation(const Angle& _rotation) override;
		Angle GetRotation() const override;
		void SetScale(const FVector2& _scale) override;
		FVector2 GetScale() const override;
	};
}
