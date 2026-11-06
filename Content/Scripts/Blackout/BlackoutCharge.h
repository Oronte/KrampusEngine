#pragma once
#include "Reflection/Reflection.h"
#include "Components/SpriteComponent.h"
#include "Components/LightComponents.h"
#include "Components/LabelComponent.h"

namespace Krampus
{

	/// <summary>
	/// A site where the attackers (Alpha) plant the charge in a network game: a circle on the floor with its letter. Place two of them in
	/// the scene (siteName "A" and "B") and set their radius.
	/// </summary>
	class BlackoutSite : public Actor
	{
		KACTOR(BlackoutSite, Actor)

	public:
		void BeginPlay() override;

		BlackoutSite(Level* _level);

		KPROPERTY(String, siteName) = "A";
		KPROPERTY_RANGE(Float, radius, 40.f, 1000.f) = 170.f;		// pixels: the charge can be planted inside

		/// <returns>True when a world position is inside the site.</returns>
		bool Contains(const FVector2& _position) const;

	protected:
		void Tick(const Float& _deltaTime) override;

	private:
		SpriteComponent*	zone		= nullptr;
		LabelComponent*		letter		= nullptr;
		float				shownRadius	= 0.f;
		std::string			shownName;

		void Refresh();
	};

	/// <summary>
	/// The buy zone of the local hero during the buy phase: a green circle ON THE FLOOR (in the world, so under the HUD and the shop)
	/// with a faint green light that shows it in the dark. Made by BlackoutGameMode, never saved.
	/// </summary>
	class BlackoutBuyZone : public Actor
	{
	public:
		BlackoutBuyZone(Level* _level);

		/// <summary>Shown around _center (radius _radius), or hidden.</summary>
		void Show(const bool& _visible, const FVector2& _center, const float& _radius);

	private:
		SpriteComponent*		ring	= nullptr;
		PointLightComponent*	glow	= nullptr;
		bool					visible	= true;
		float					radius	= 0.f;
	};

	/// <summary>
	/// The charge: Bomb.png with a red light, seen where it lies (dropped or planted). Made by BlackoutGameMode in a network game,
	/// never saved. Planted, it blinks faster and faster until it explodes.
	/// </summary>
	class BlackoutCharge : public Actor
	{
	public:
		BlackoutCharge(Level* _level);

		/// <summary>On the floor at _position, or hidden (carried).</summary>
		void Show(const bool& _visible, const FVector2& _position);

		/// <summary>Planted: blinks every _period seconds.</summary>
		void SetBlink(const float& _period);

	protected:
		void Tick(const Float& _deltaTime) override;

	private:
		SpriteComponent*		box		= nullptr;
		PointLightComponent*	light	= nullptr;
		bool					visible	= false;
		float					period	= 0.f;		// 0: a steady light
		float					clock	= 0.f;
	};

}
