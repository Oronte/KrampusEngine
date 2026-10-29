#pragma once
#include "Component.h"
#include "Utilities/System/Debuggable.h"

namespace Krampus
{

	/// <summary>
	/// Component that represents the 2D/3D audio listener in the scene, updating the global audio listener position and orientation.
	/// </summary>
	class AudioListenerComponent : public Component, public IDebuggable
	{
		Bool		topView			= false;
		Bool		active			= true;

		FVector2	cachedPosition	= FVector2::Zero();
		FVector2	cachedForward	= FVector2::Zero();
		FVector2	cachedUp		= FVector2::Zero();
		Bool		dirty			= true;

	public:
		/// <summary>
		/// Sets the master volume for all audio playback in the range [0, 100].
		/// </summary>
		/// <param name="_volume">The global volume level.</param>
		static inline void SetGlobalVolume(const Float& _volume)
		{
			sf::Listener::setGlobalVolume(_volume);
		}

		/// <summary>
		/// Gets the master audio volume level.
		/// </summary>
		/// <returns>Current global volume level.</returns>
		static inline Float GetGlobalVolume()
		{
			return sf::Listener::getGlobalVolume();
		}

		/// <summary>
		/// Sets the directional hearing cone of the listener.
		/// </summary>
		/// <param name="_cone">Listener cone specification.</param>
		inline void SetCone(const sf::Listener::Cone& _cone)
		{
			sf::Listener::setCone(_cone);
		}

		/// <summary>
		/// Gets the directional hearing cone of the listener.
		/// </summary>
		/// <returns>Current listener cone.</returns>
		inline sf::Listener::Cone GetCone() const
		{
			return sf::Listener::getCone();
		}

		/// <summary>
		/// Activates or deactivates this audio listener.
		/// </summary>
		/// <param name="_active">True to activate, false to deactivate.</param>
		void SetActive(const Bool& _active);

		/// <summary>
		/// Checks whether this audio listener is currently active.
		/// </summary>
		/// <returns>True if active, false otherwise.</returns>
		inline Bool IsActive() const { return active; }

		/// <summary>
		/// Sets whether the listener is viewed from a top-down perspective (Z is up) or side view (Y is up).
		/// </summary>
		/// <param name="_topView">True for top-down perspective.</param>
		inline void		SetTopView(const Bool& _topView)	{ topView = _topView; dirty = true; }

		/// <summary>
		/// Checks whether the listener uses a top-down orientation.
		/// </summary>
		/// <returns>True if top view, false if side view.</returns>
		inline Bool		IsTopView() const					{ return topView; }

		/// <summary>
		/// Marks the cached spatial values as dirty, forcing an update on next tick.
		/// </summary>
		inline void		MarkDirty() { dirty = true; }

	public:
		/// <summary>
		/// Constructs an AudioListenerComponent attached to the specified actor.
		/// </summary>
		/// <param name="_owner">Pointer to the owning Actor.</param>
		/// <param name="_topView">Whether top-down perspective is enabled.</param>
		AudioListenerComponent(Actor* _owner, const Bool& _topView = false);

		/// <summary>
		/// Lifecycle callback invoked when gameplay begins.
		/// </summary>
		virtual void BeginPlay()					override;

		/// <summary>
		/// Updates the global audio listener position and orientation each frame based on owner transform.
		/// </summary>
		/// <param name="_deltaTime">Elapsed frame time in seconds.</param>
		virtual void Tick(const Float& _deltaTime)	override;

	private:
		void DrawDebug() override;	// the hearing cone
		void UpdateListener();
		Bool HasChanged(const FVector2& _position, const FVector2& _forward, const FVector2& _up) const;

	public:
		/// <summary>
		/// Returns a string representation of the component.
		/// </summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;
	};

}
