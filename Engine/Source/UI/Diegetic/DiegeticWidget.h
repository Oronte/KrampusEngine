#pragma once
#include "Actors/Actor.h"

namespace Krampus
{

	class Level;

	/// <summary>
	/// Base actor for diegetic UI elements rendered in world-space or anchored to screen coordinates with camera tracking.
	/// </summary>
	class DiegeticWidget : public Actor
	{
		Event<sf::View>::ListenerHandle		onViewChangeHandle;

	protected:
		FVector2							screenPosition;
		Angle								localRotation;

	protected:
		/// <summary>Gets the assigned screen position anchor.</summary>
		/// <returns>Screen position coordinates.</returns>
		inline FVector2 GetScreenPosition() const
		{
			return screenPosition;
		}

		/// <summary>Gets the local rotation angle.</summary>
		/// <returns>Rotation angle.</returns>
		inline Angle GetLocalRotation() const
		{
			return localRotation;
		}

		/// <summary>Constructs a DiegeticWidget in the given level.</summary>
		/// <param name="_level">Pointer to owning Level.</param>
		DiegeticWidget(Level* _level);

	public:
		/// <summary>Sets the screen position anchor coordinates and re-projects world position.</summary>
		/// <param name="_screenPos">Screen anchor position.</param>
		void SetScreenPosition(const FVector2& _screenPos);

		/// <summary>Sets the local rotation angle.</summary>
		/// <param name="_rotation">Rotation angle.</param>
		void SetLocalRotation(const Angle& _rotation);

		/// <summary>Destroys this diegetic widget.</summary>
		virtual void Destroy() override final;

	private:
		void UpdateWorldPos(const sf::View& _view);

	public:
		/// <summary>Returns a string representation of the diegetic widget.</summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;

		friend class HUD;
	};

}
