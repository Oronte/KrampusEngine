#pragma once
#include "Actors/Actor.h"

namespace Krampus
{

	class Level;

	/// <summary>
	/// Base actor for diegetic UI elements: an actor of the world kept at a place of the screen (screenPosition, in window pixels),
	/// whatever the camera does. Spawn one like any actor (editor: Add Actor > DiegeticImageWidget / DiegeticLabelWidget /
	/// DiegeticButtonWidget, or in code: GetLevel()->SpawnDiegeticWidget<DiegeticLabelWidget>(FVector2(40, 40), TextData("Hi"))).
	/// A class of your own can derive from it or from one of them (Content panel > + New Script).
	/// </summary>
	class DiegeticWidget : public Actor
	{
		Event<sf::View>::ListenerHandle		onViewChangeHandle;

	protected:
		FVector2							screenPosition;
		Angle								localRotation;

		/// <summary>Constructs a DiegeticWidget in the given level.</summary>
		/// <param name="_level">Pointer to owning Level.</param>
		DiegeticWidget(Level* _level);

	public:
		/// <summary>Gets the assigned screen position anchor (window pixels).</summary>
		inline FVector2 GetScreenPosition() const
		{
			return screenPosition;
		}

		/// <summary>Gets the local rotation angle.</summary>
		inline Angle GetLocalRotation() const
		{
			return localRotation;
		}

		/// <summary>Sets the screen position anchor coordinates and re-projects world position.</summary>
		/// <param name="_screenPos">Screen anchor position.</param>
		void SetScreenPosition(const FVector2& _screenPos);

		/// <summary>Sets the local rotation angle.</summary>
		/// <param name="_rotation">Rotation angle.</param>
		void SetLocalRotation(const Angle& _rotation);

	protected:
		void BeginDestroy() override;

	private:
		void UpdateWorldPos(const sf::View& _view);

	public:
		/// <summary>Returns a string representation of the diegetic widget.</summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;

		friend class HUD;
	};

}
