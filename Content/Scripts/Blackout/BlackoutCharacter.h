#pragma once
#include "Reflection/Reflection.h"
#include "Components/SpriteComponent.h"
#include "Components/PhysicsComponent.h"
#include "Components/LightComponents.h"
#include "Graphics/Particles/ParticleEmitter.h"

// Blackout: a top-down shooter in the dark (scene Blackout, Tutorial chapter 25).
//   - The level is black: the hero only sees what his flashlight (a SpotLightComponent stopped by the walls) and his aura light.
//   - The enemies walk in the dark: their steps and their shots make "echoes" (rings on the HUD) where they are.
//   - Bullets are actors that raycast from one frame to the next (WorldPhysics::Raycast): walls stop them.

namespace Krampus
{

	class ParticleComponent;

	/// <returns>A random number in [_min, _max].</returns>
	float BlackoutRandom(const float& _min, const float& _max);

	/// <summary>
	/// The guns, bought in the shop at the start of a round. A soldier carries two: a primary (shotgun, rifle, sniper; shared value
	/// "primary_<player id>", -1 none) and a sidearm (pistols; "sidearm_<player id>", 0 the free pistol). 1 / 2 take one or the other.
	/// A network player who is still alive at the end of a round keeps his guns.
	/// </summary>
	struct BlackoutWeapon
	{
		enum Category { Sidearm = 0, Shotgun = 1, Rifle = 2, Sniper = 3 };

		std::string	name;
		float		damage		= 20.f;		// per bullet
		float		delay		= 0.1f;		// seconds between two shots
		float		spread		= 2.f;		// degrees, each side
		int			pellets		= 1;		// bullets per shot
		int			magazine	= 20;
		float		reload		= 2.f;		// seconds
		float		speed		= 1500.f;	// pixels per second
		float		range		= 2000.f;	// pixels a bullet flies
		bool		automatic	= true;		// keep the button down to keep shooting
		float		view		= 1440.f;	// width of the world the camera shows
		int			price		= 0;		// credits in the shop
		std::string	texture;				// seen from above (in the hands, on the floor): Content/Textures/<texture>.png, muzzle on the right
		std::string	icon;					// seen from the side (shop, HUD)
		FVector2	size;					// pixels in the world, in the hands
		float		grip		= 0.f;		// pixels from the soldier's head (forward) to the back of the gun
		int			category	= Sidearm;
		Color		tint		= Color::White();	// the same picture for several guns: a color tells them apart

		inline bool IsSidearm() const { return category == Sidearm; }

		static constexpr int	COUNT		= 11;
		static constexpr int	PISTOL		= 0;		// the free sidearm
		static constexpr int	GRENADE_PRICE	= 300;
		static constexpr int	MAX_GRENADES	= 3;
		static constexpr int	FLASH_PRICE		= 250;
		static constexpr int	MAX_FLASHES		= 2;
		static constexpr int	SMOKE_PRICE		= 300;
		static constexpr int	MAX_SMOKES		= 2;
		static constexpr int	LIGHT_SHIELD_PRICE	= 400;
		static constexpr int	HEAVY_SHIELD_PRICE	= 1000;
		static constexpr float	LIGHT_SHIELD		= 25.f;		// points of damage taken by the shield before the health
		static constexpr float	HEAVY_SHIELD		= 50.f;
		static const BlackoutWeapon& Get(const int& _index);
		/// <returns>The name of a category, for the shop ("PISTOLETS"...).</returns>
		static std::string CategoryName(const int& _category);
	};

	/// <summary>A particle effect that plays once and removes itself (Level::SpawnParticles keeps the looping presets, Sparks and Smoke, forever).</summary>
	ParticleComponent* BlackoutEffect(Level* _level, const ParticlePreset& _preset, const FVector2& _position);

	/// <summary>
	/// Blood of a hit at _position, going along _direction: drops that mark the floor where they stop, a splash on the floor and, when a
	/// wall is close behind, a splash on it (the actors with a StainComponent: BlackoutFloor, BlackoutWall). _amount: 1 a hit, more a death.
	/// </summary>
	void BlackoutBleed(Level* _level, const Actor* _victim, const FVector2& _position, const FVector2& _direction, const float& _amount);

	/// <summary>The gun in the hands of a soldier (a second sprite: an actor has one component of each class). Made at run time only.</summary>
	class BlackoutGunComponent : public SpriteComponent
	{
	public:
		using SpriteComponent::SpriteComponent;
	};

	/// <summary>
	/// What the hero and the enemies share: a soldier sprite (Content/Textures/Player.png, turned toward where it looks) with a gun in
	/// his hands (BlackoutGunComponent, SetGunLook), a round dynamic body without gravity, health.
	/// </summary>
	class BlackoutCharacter : public Actor
	{
		KCLASS(BlackoutCharacter, Actor)

	public:
		void BeginPlay() override;

		BlackoutCharacter(Level* _level);

		KPROPERTY_RANGE(Float, maxHealth, 1.f, 1000.f) = 100.f;
		KPROPERTY_RANGE(Float, moveSpeed, 0.f, 1000.f) = 260.f;		// pixels per second
		KPROPERTY_RANGE(Float, bulletDamage, 0.f, 500.f) = 25.f;
		KPROPERTY_RANGE(Float, fireDelay, 0.02f, 5.f) = 0.16f;		// seconds between two shots

		inline int GetTeam() const { return team; }
		inline void SetTeam(const int& _team) { team = _team; }
		/// <returns>The network player that controls it (BlackoutHero in a network game), -1 for the others.</returns>
		virtual int GetPlayerId() const { return -1; }
		inline float GetHealth() const { return health; }
		inline bool IsDead() const { return health <= 0.f; }
		inline float GetBulletRange() const { return bulletRange; }

		/// <returns>Where its bullets leave from: the end of the gun (the head when the gun is inside a wall).</returns>
		FVector2 MuzzlePosition();

		/// <summary>Turns so that the GUN (on the right of the head) points at _target, not the head.</summary>
		void AimAt(const FVector2& _target);
		/// <returns>Where a bullet aimed at _target goes: from the muzzle to it, or straight ahead when _target is not in front of the
		/// muzzle (the mouse between the head and the gun).</returns>
		FVector2 AimDirection(const FVector2& _target);

		/// <summary>Shield points (bought): they take the damage before the health. Set on every copy (the same hits everywhere).</summary>
		inline void SetShield(const float& _shield) { shield = std::max(0.f, _shield); }
		inline float GetShield() const { return shield; }

		/// <summary>0..1: how much of him is drawn (an enemy nobody of our team sees is not drawn, even in the dim light).</summary>
		void SetShown(const float& _shown);
		inline float GetShown() const { return shown; }

		/// <summary>Puts a gun of BlackoutWeapon in his hands (only its picture: the shots are set apart). Hidden once dead.</summary>
		void SetGunLook(const int& _weapon);
		inline int GetGunLook() const { return gunLook; }

		/// <summary>A bullet hit it. _from: where the shot came from.</summary>
		virtual void TakeDamage(const float& _amount, const FVector2& _from);

		/// <returns>True when nothing (wall, other body) is between this one and _other, closer than _range.</returns>
		bool CanSee(const Actor* _other, const float& _range);

	protected:
		SpriteComponent*	body			= nullptr;
		PhysicsComponent*	physics			= nullptr;
		int					team			= 0;		// solo: 0 hero, 1 enemies; network game: 0 Alpha, 1 Bravo. No friendly fire
		float				health			= 0.f;
		float				fireCooldown	= 0.f;
		float				hurtFlash		= 0.f;		// > 0: the sprite is white for a moment
		Color				tint			= Color::White();
		float				shield			= 0.f;
		float				shown			= 1.f;
		int					pellets			= 1;		// bullets per shot (spread over the aim)
		float				bulletSpeed		= 1500.f;
		float				bulletRange		= 2100.f;
		FVector2			lastMuzzle;					// of the last Fire that shot
		std::vector<FVector2>	lastDirections;		// one per pellet
		BlackoutGunComponent*	gun			= nullptr;	// made by SetGunLook
		int					gunLook			= 0;

		bool				gunHidden		= false;

		/// <summary>The gun leaves the hands (dead: it lies on the floor, BlackoutDroppedWeapon).</summary>
		void HideGun();
		/// <summary>Colors of the body and the gun again (tint, hit, how much he is shown).</summary>
		void Repaint();

		void Tick(const Float& _deltaTime) override;

		/// <summary>Moves with the physics (the walls stop it): a direction of length 0..1, in pixels per second.</summary>
		void Walk(const FVector2& _direction, const float& _speed);

		/// <summary>Shoots a bullet toward _target, turned by a random angle up to _spread degrees. False while the gun cools down.</summary>
		bool Fire(const FVector2& _target, const float& _spread);

		/// <summary>Health reached 0.</summary>
		virtual void Die() { }

	private:
		/// <summary>The pivot of the soldier picture is its head (the gun goes to the right, rotation 0).</summary>
		void PlaceSprite();
	};

	/// <summary>
	/// A bullet: a short bright line with a small light, moved every frame; what is between the old and the new position is hit
	/// (raycast). Made by BlackoutCharacter::Fire, never saved in a scene.
	/// </summary>
	class BlackoutBullet : public Actor
	{
	public:
		BlackoutBullet(Level* _level);

		/// <summary>_real false: the copy of a shot made on another machine, only seen (that machine tells the hits).</summary>
		void Launch(const BlackoutCharacter& _shooter, const FVector2& _direction, const float& _damage, const bool& _real = true,
			const float& _speed = 1500.f, const float& _range = 2100.f);

		/// <summary>Pixels its light reaches: a soldier of the other side this close (no wall in between) is seen (BlackoutGameMode::UpdateFog).</summary>
		static constexpr float	LIGHT_RADIUS	= 120.f;
		int GetTeam() const { return team; }

	protected:
		void Tick(const Float& _deltaTime) override;

	private:
		SpriteComponent*		line		= nullptr;
		PointLightComponent*	glow		= nullptr;
		FVector2				velocity;
		float					damage		= 0.f;
		float					life		= 1.4f;		// seconds
		std::uint64_t			shooterId	= 0;		// an id, not a pointer: the shooter may die before the bullet
		int						team		= 0;
		int						shooterPlayer	= -1;	// network player who shot (credits for a kill)
		bool					real		= true;
		FVector2				origin;
	};

	/// <summary>
	/// A gun on the floor: dropped by a soldier who died (or swapped), taken with E (BlackoutGameMode::UpdatePickup). A faint light
	/// shows it in the dark. Made by BlackoutGameMode (the host decides, BO_Drop / BO_Picked), never saved.
	/// </summary>
	class BlackoutDroppedWeapon : public Actor
	{
	public:
		BlackoutDroppedWeapon(Level* _level);

		/// <summary>_dropId: the same on every machine. _angle: degrees, how it lies.</summary>
		void Setup(const int& _dropId, const int& _weapon, const float& _angle);
		inline int GetDropId() const { return dropId; }
		inline int GetWeapon() const { return weapon; }

	private:
		SpriteComponent*		picture		= nullptr;
		PointLightComponent*	glow		= nullptr;
		int						dropId		= -1;
		int						weapon		= 0;
	};

}
