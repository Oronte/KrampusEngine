#pragma once
#include "Core/GameObject.h"
#include "Utilities/System/Transformable.h"
#include "Utilities/System/Timer.h"

namespace Krampus
{
	class Engine;

	/// <summary>
	/// Base class for components that can be attached to actors to extend their behavior, transform, and rendering.
	/// </summary>
	class Component : public IGameObject, public ITransformable
	{
	protected:
		class Actor*	owner;

	public:
		/// <summary>
		/// Gets the actor that owns this component.
		/// </summary>
		/// <returns>Pointer to the owning Actor.</returns>
		inline Actor* GetOwner() const
		{
			return owner;
		}

		/// <summary>
		/// Gets the Level containing the owning actor.
		/// </summary>
		/// <returns>Pointer to the Level.</returns>
		class Level* GetLevel() const;

		/// <summary>
		/// Gets the Engine world instance.
		/// </summary>
		/// <returns>Pointer to the Engine.</returns>
		Engine* GetWorld() const;

		/// <summary>
		/// Gets the elapsed time of the current frame in seconds.
		/// </summary>
		/// <returns>Delta time as Float.</returns>
		Float GetDeltaTime() const;

	protected:
		/// <summary>
		/// Constructs a Component with the specified owning actor.
		/// </summary>
		/// <param name="_owner">Pointer to the owning Actor.</param>
		Component(Actor* _owner);

		/// <summary>
		/// Creates and starts a timer managed by the level's TimerManager.
		/// </summary>
		/// <param name="_callback">Callback function triggered on timeout.</param>
		/// <param name="_duration">Timer duration in seconds.</param>
		/// <param name="_isLoop">Whether the timer automatically repeats.</param>
		/// <param name="_startRunning">Whether the timer should start immediately.</param>
		/// <returns>Pointer to the created Timer.</returns>
		Timer* CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);

	public:
		/// <summary>
		/// Virtual destructor for Component.
		/// </summary>
		virtual ~Component() = default;

	public:
		/// <summary>
		/// Lifecycle callback invoked when the component is created and initialized.
		/// </summary>
		virtual void Construct() override;

		/// <summary>
		/// Lifecycle callback invoked when the component is removed or deconstructed.
		/// </summary>
		virtual void Deconstruct() override;

		/// <summary>
		/// Lifecycle callback invoked when gameplay begins for this component.
		/// </summary>
		virtual void BeginPlay() override {};

		/// <summary>
		/// Updates the component each frame.
		/// </summary>
		/// <param name="_deltaTime">Time elapsed since the previous frame in seconds.</param>
		virtual void Tick(const Float& _deltaTime) override {};

		/// <summary>
		/// Lifecycle callback invoked just before the component is destroyed.
		/// </summary>
		virtual void BeginDestroy() override {};

		/// <summary>
		/// Returns a human-readable string representation of the component.
		/// </summary>
		/// <returns>Component state as std::string.</returns>
		virtual std::string ToString() const override;

		/// <summary>
		/// Sets the position of the component relative to its owner.
		/// </summary>
		/// <param name="_newPosition">The new 2D position.</param>
		void SetPosition(const FVector2& _newPosition) override;

		/// <summary>
		/// Gets the position of the component.
		/// </summary>
		/// <returns>Position as FVector2.</returns>
		FVector2 GetPosition() const override;

		/// <summary>
		/// Sets the rotation angle of the component.
		/// </summary>
		/// <param name="_newRotation">The new Angle.</param>
		void SetRotation(const Angle& _newRotation) override;

		/// <summary>
		/// Gets the rotation angle of the component.
		/// </summary>
		/// <returns>Current Angle.</returns>
		Angle GetRotation() const override;

		/// <summary>
		/// Sets the 2D scale factor of the component.
		/// </summary>
		/// <param name="_newScale">The new scale factor.</param>
		void SetScale(const FVector2& _newScale) override;

		/// <summary>
		/// Gets the 2D scale factor of the component.
		/// </summary>
		/// <returns>Current scale as FVector2.</returns>
		FVector2 GetScale() const override;
	};

}
