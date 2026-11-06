#pragma once
#include "BlackoutCharacter.h"
#include "Components/CameraComponent.h"

namespace Krampus
{

	/// <summary>
	/// The player of Blackout: ZQSD / WASD / arrows to run, Shift to walk (silent), the mouse to aim, left click to shoot, R to reload,
	/// right click or G to throw a grenade, F a flash, C a smoke.
	/// Carries the camera, a flashlight (cone stopped by the walls) and a small light around him.
	/// During the buy phase he stays in a small zone around where he started (BlackoutGameMode::buyZoneRadius) and does not shoot.
	/// In a network game (BlackoutGameMode::IsVersus) every machine has one hero per player (SetupPlayer): only the local one reads the
	/// keyboard; the others follow their NetworkSyncComponent. The lights of the other team are off: they are only seen in our lights.
	/// </summary>
	class BlackoutHero : public BlackoutCharacter
	{
		KACTOR(BlackoutHero, BlackoutCharacter)

	public:
		void BeginPlay() override;

		BlackoutHero(Level* _level);

		static constexpr float	WALK_FACTOR		= 0.45f;	// Shift: speed while walking (silent)
		static constexpr float	STEP_INTERVAL	= 0.38f;	// seconds between two noisy steps while running
		// Moving spoils the aim: the spread is spread * factor + extra degrees (standing: 1, 0)
		static constexpr float	WALK_SPREAD		= 1.6f;
		static constexpr float	WALK_SPREAD_ADD	= 1.f;
		static constexpr float	RUN_SPREAD		= 2.6f;
		static constexpr float	RUN_SPREAD_ADD	= 3.f;

		/// <summary>His two guns (BlackoutWeapon::Get; _primary -1: none, he holds the sidearm). Full magazines, the primary in hand.</summary>
		void SetLoadout(const int& _primary, const int& _sidearm);
		/// <summary>A gun bought or picked up: it takes the place of the one of its kind (primary or sidearm) and comes in hand.</summary>
		void GiveWeapon(const int& _weapon);
		/// <summary>Takes in hand slot 0 (primary, if he has one) or 1 (sidearm). Each one keeps its ammo.</summary>
		void Equip(const int& _slot);
		inline int GetPrimary() const { return primary; }
		inline int GetSidearm() const { return sidearm; }
		inline int GetSlot() const { return slot; }
		/// <summary>One more grenade (bought), up to BlackoutWeapon::MAX_GRENADES.</summary>
		void AddGrenade();
		/// <summary>One more flash (bought), up to BlackoutWeapon::MAX_FLASHES.</summary>
		void AddFlash();
		inline bool IsWalking() const { return walking; }
		inline int GetWeapon() const { return weapon; }
		inline int GetMagazine() const { return magazineSize; }
		inline int GetAmmo() const { return ammo; }
		inline bool IsReloading() const { return reloadLeft > 0.f; }
		inline int GetGrenades() const { return grenadesLeft; }
		inline int GetFlashes() const { return flashesLeft; }
		/// <summary>One more smoke (bought), up to BlackoutWeapon::MAX_SMOKES.</summary>
		void AddSmoke() { smokesLeft = std::min(smokesLeft + 1, BlackoutWeapon::MAX_SMOKES); }
		inline int GetSmokes() const { return smokesLeft; }
		inline bool IsLocalPlayer() const { return localPlayer; }
		/// <returns>Degrees a bullet can turn from the aim, either side (the red cone of the HUD): the gun's, more while moving.</returns>
		inline float GetSpread() const { return spread * moveSpread + moveSpreadAdd; }
		/// <returns>Where a bullet aimed at the mouse goes (BlackoutCharacter::AimDirection).</returns>
		inline FVector2 GetAimDirection() { return AimDirection(MouseInWorld()); }
		/// <returns>Where the local player aims: the mouse in the world.</returns>
		inline FVector2 GetAimPoint() const { return MouseInWorld(); }
		/// <returns>Half the angle of the flashlight, in degrees, and how far it lights.</returns>
		inline float GetLightHalfAngle() const { return static_cast<float>(flashlight->angle) / 2.f; }
		inline float GetLightRange() const { return static_cast<float>(flashlight->radius); }
		/// <returns>Where the hero started the round: the middle of his buy zone.</returns>
		inline FVector2 GetSpawnPosition() const { return spawnPosition; }
		int GetPlayerId() const override { return playerId; }

		/// <summary>
		/// Network game: the hero of _playerId, in _team. _local: this machine plays it (camera, keyboard). _friend: same team as the local
		/// player (its lights show what it sees; an enemy's lights are off and its steps make echoes).
		/// </summary>
		void SetupPlayer(const int& _playerId, const int& _team, const bool& _local, const bool& _friend);

		void TakeDamage(const float& _amount, const FVector2& _from) override;

		/// <summary>The player left the game: the hero falls without bleeding.</summary>
		void Abandon();

		static Color TeamColor(const int& _team);

	protected:
		void Tick(const Float& _deltaTime) override;
		void Die() override;

	private:
		CameraComponent*		camera		= nullptr;
		SpotLightComponent*		flashlight	= nullptr;
		PointLightComponent*	aura		= nullptr;
		int						ammo		= 0;
		float					reloadLeft	= 0.f;
		int						weapon			= 0;		// the one in hand
		int						primary			= -1;
		int						sidearm			= BlackoutWeapon::PISTOL;
		int						slot			= 1;		// 0 primary, 1 sidearm
		int						slotAmmo[2]		= { 0, 0 };
		bool					slotKeyHeld		= true;
		float					moveSpread		= 1.f;		// smoothed
		float					moveSpreadAdd	= 0.f;
		int						magazineSize	= 25;
		float					reloadSeconds	= 2.f;
		float					spread			= 3.f;
		bool					automatic		= true;
		bool					fireHeld		= false;
		bool					triggerReady	= false;	// the button was up once since the start
		int						grenadesLeft	= 0;
		int						flashesLeft		= 0;
		float					throwCooldown	= 0.f;
		bool					throwHeld		= true;		// true: a button already held at the start does not throw
		bool					flashHeld		= true;
		int						smokesLeft		= 0;
		bool					smokeHeld		= true;
		int						playerId		= -1;		// -1: solo
		bool					localPlayer		= true;
		bool					friendly		= true;
		float					stepTime		= 0.f;
		bool					walking			= false;	// Shift is down
		FVector2				spawnPosition;

		/// <returns>The mouse in the world, through the hero's camera.</returns>
		FVector2 MouseInWorld() const;
		/// <summary>The stats, the picture and the camera of the gun in hand.</summary>
		void ApplyStats(const int& _weapon);
		/// <summary>The local player pressed 1 or 2: takes that gun and tells the others (BO_Slot).</summary>
		void SwitchSlot(const int& _slot);
		void Reload();
		/// <summary>_kind: BlackoutGrenade::Kind (frag, flash, smoke).</summary>
		void ThrowGrenade(const FVector2& _target, const int& _kind);
		void Step();
		/// <summary>During the buy phase: the direction loses what would take the hero out of his zone (and he is put back in it).</summary>
		FVector2 KeepInBuyZone(const FVector2& _direction, const float& _radius);
	};

}
