#pragma once
#include "Reflection/Reflection.h"
#include "Components/LightComponents.h"
#include "UI/Widgets/BasicWidgets.h"
#include "Network/NetworkManager.h"
#include "BlackoutCharacter.h"
#include <map>

namespace Krampus
{
	class BlackoutCharacter;
	class BlackoutHero;
	class BlackoutSite;
	class BlackoutCharge;
	class BlackoutChat;
	class BlackoutDroppedWeapon;

	/// <summary>
	/// A ring on the HUD (a circle outline, not lit by the 2D lights): what an echo is drawn with.
	/// </summary>
	class BlackoutRingWidget : public Widget
	{
	public:
		BlackoutRingWidget(Level* _level);

		ImageWidgetComponent* GetRing() const { return ring; }

	private:
		ImageWidgetComponent*	ring	= nullptr;
	};

	/// <summary>
	/// The rules of Blackout. One in the scene: its GlobalLightComponent makes the level dark, it draws the HUD (health, ammo,
	/// messages), the echoes of the steps and shots, and ends the round. Escape goes back to the menu.
	///   - Solo (offline): the hero and the BlackoutEnemy of the scene. Every enemy dead: won, hero dead: lost; Enter or R plays again.
	///   - Versus (a network game started from the lobby): the hero and the enemies of the scene are removed, every machine makes one
	///     hero per player (team from the shared value "team_<id>", spawn points of its team). Messages: BO_Shot / BO_Grenade (copies
	///     of a shot, seen only), BO_Hit (the shooter's machine tells a hit to everybody), BO_Start (everybody opens a scene).
	///     The charge: ALPHA attacks (it lies on the floor in front of their corner at the start of the round, an attacker walks on it to
	///     take it, an icon shows the carrier to his team; E on a BlackoutSite to plant it), BRAVO defends (E near the planted charge
	///     to defuse it). Names over the heads: allies blue, enemies red while seen. The local hero has a red cone: his spread. The host decides (BO_Charge: its state to everybody; BO_Plant / BO_Defuse: a player asks; BO_RoundEnd: who won)
	///     and starts the next round 5 s after the end. A chat (BlackoutChat) is there too.
	/// </summary>
	class BlackoutGameMode : public Actor
	{
		KACTOR(BlackoutGameMode, Actor)

	public:
		void BeginPlay() override;

		BlackoutGameMode(Level* _level);

		KPROPERTY(String, sceneName) = "Blackout";						// opened again to play another round
		KPROPERTY(String, menuScene) = "BlackoutMenu";					// opened by Escape
		KPROPERTY(String, lobbyScene) = "BlackoutLobby";				// opened at the end of a match
		KPROPERTY(FVector2, arenaSize) = FVector2(3200.f, 2200.f);		// around this actor: where the enemies wander
		KPROPERTY_RANGE(Float, hearingRange, 0.f, 5000.f) = 1100.f;		// a shot alerts the enemies this close
		KPROPERTY_RANGE(Float, roundTime, 20.f, 900.f) = 120.f;			// versus: seconds to plant the charge
		KPROPERTY_RANGE(Float, detonateTime, 5.f, 300.f) = 40.f;		// versus: seconds between the plant and the explosion
		KPROPERTY_RANGE(Float, plantTime, 0.5f, 30.f) = 4.f;			// versus: seconds to hold E to plant
		KPROPERTY_RANGE(Float, defuseTime, 0.5f, 30.f) = 7.f;			// versus: seconds to hold E to defuse
		KPROPERTY_RANGE(Float, buyTime, 0.f, 120.f) = 15.f;				// seconds of shopping at the start of a round (nobody moves)
		KPROPERTY_RANGE(Int, soloCredits, 0, 20000) = 5000;				// credits to shop with in solo
		KPROPERTY_RANGE(Float, buyZoneRadius, 40.f, 2000.f) = 260.f;		// pixels around his start a hero stays in during the buy phase

		// Credits (shared value "credits_<id>", kept by the host)
		static constexpr int	START_CREDITS	= 800;
		static constexpr int	MAX_CREDITS		= 9000;
		static constexpr int	WIN_CREDITS		= 3000;
		static constexpr int	LOSS_CREDITS	= 1900;
		static constexpr int	KILL_CREDITS	= 200;
		// The shop: the guns (BlackoutWeapon::Get 0..COUNT-1), then these
		static constexpr int	ITEM_LIGHT_SHIELD	= BlackoutWeapon::COUNT;
		static constexpr int	ITEM_HEAVY_SHIELD	= BlackoutWeapon::COUNT + 1;
		static constexpr int	ITEM_GRENADE		= BlackoutWeapon::COUNT + 2;
		static constexpr int	ITEM_FLASH			= BlackoutWeapon::COUNT + 3;
		static constexpr int	ITEM_SMOKE			= BlackoutWeapon::COUNT + 4;
		static constexpr int	SHOP_ITEMS			= BlackoutWeapon::COUNT + 5;

		/// <summary>Runs a button command of the HUD (the shop: "buy<item>"). Called by BlackoutButton.</summary>
		void Command(const std::string& _command);

		/// <returns>True during the shopping time at the start of a round.</returns>
		inline bool IsBuyPhase() const { return buyLeft > 0.f; }

		/// <summary>The local hero ran a step: a faint ring for him; in solo the enemies close enough hear it.</summary>
		void HeroStep(const BlackoutCharacter& _hero);

		/// <summary>Network id of the hero of a player: HERO_NETWORK_ID + player id (the same on every machine).</summary>
		static constexpr int	HERO_NETWORK_ID		= 0x30000000;
		/// <summary>A match: the first team to win ROUNDS_TO_WIN rounds. From round SWAP_ROUND on, the teams swap sides (attack / defense).</summary>
		static constexpr int	ROUNDS_TO_WIN		= 13;
		static constexpr int	SWAP_ROUND			= 13;

		/// <returns>The team that carries and plants the charge in a round (Alpha first, Bravo after the swap).</returns>
		static int AttackersOf(const int& _round) { return _round < SWAP_ROUND ? 0 : 1; }
		inline int GetAttackers() const { return attackers; }

		/// <returns>The game mode of the level, or nullptr.</returns>
		static BlackoutGameMode* Of(Level* _level);

		/// <returns>The team of a network player: the shared value "team_<id>" (0 Alpha, 1 Bravo), else id % 2.</returns>
		static int TeamOf(const NetworkManager* _net, const int& _playerId);

		/// <returns>Name of a team: "ALPHA" or "BRAVO".</returns>
		static std::string TeamName(const int& _team);

		/// <returns>True in a network game (two teams of players).</returns>
		inline bool IsVersus() const { return versus; }

		/// <returns>The team of the player of this machine (0 in solo).</returns>
		inline int GetLocalTeam() const { return localTeam; }

		/// <returns>True while the local hero must not move (typing in the chat, planting or defusing).</returns>
		bool IsMoveBlocked() const;
		/// <returns>True while the local hero must not shoot nor throw (the same, and the buy phase, the shop).</returns>
		bool IsFireBlocked() const;

		/// <summary>A flash in the eyes of the local player: the screen is white for _seconds (fading at the end).</summary>
		void Blind(const float& _seconds);

		/// <returns>The hero this machine plays (alive or not), or nullptr.</returns>
		BlackoutHero* GetHero();

		/// <returns>The hero of a network player, or nullptr.</returns>
		BlackoutHero* FindHero(const int& _playerId);

		/// <summary>Versus: tells every machine that the hero of _playerId is hurt (each one hurts its copy).</summary>
		void SendHit(const int& _playerId, const float& _damage, const FVector2& _from, const int& _attacker = -1);

		/// <summary>A ring on the HUD at a world position (a step, a shot), seen even in the dark.</summary>
		void Echo(const FVector2& _position, const Color& _color, const float& _size);

		/// <summary>Someone fired: an echo for the shots of the other side, and in solo the enemies near the hero's shots come to look.</summary>
		void Shot(const BlackoutCharacter& _shooter, const FVector2& _position);

		/// <summary>A grenade exploded: a big echo, and the enemies close enough come to look.</summary>
		void Explosion(const FVector2& _position);

		/// <summary>The hero was hit: the screen flashes red.</summary>
		void HeroHurt();

		/// <summary>An enemy died: its gun (_weapon) falls on the floor.</summary>
		void EnemyKilled(const FVector2& _position, const int& _weapon);

		/// <returns>Credits given back for the gun of _player in _slot (0 primary, 1 sidearm) when he buys another one of its kind: all of
		/// it when it was bought in this round, half otherwise, nothing for the free pistol.</returns>
		int SellValue(const int& _player, const int& _slot);

		/// <returns>A random point of the arena, at least _margin from its sides.</returns>
		FVector2 RandomPoint(const float& _margin) const;

	protected:
		void Tick(const Float& _deltaTime) override;

	private:
		struct EchoRing
		{
			BlackoutRingWidget*	widget	= nullptr;
			FVector2			position;
			Color				color;
			float				size	= 0.f;
			float				age		= 1.f;		// 0..1, 1 = gone
		};

		enum ChargeState { Carried = 0, Dropped = 1, Planted = 2, Defused = 3, Exploded = 4 };

		GlobalLightComponent*	ambient			= nullptr;
		std::vector<EchoRing>	echoes;
		size_t					nextEcho		= 0;
		TextWidget*				statusText		= nullptr;
		TextWidget*				ammoText		= nullptr;
		TextWidget*				grenadeText		= nullptr;
		TextWidget*				messageText		= nullptr;
		TextWidget*				helpText		= nullptr;
		TextWidget*				timerText		= nullptr;
		TextWidget*				objectiveText	= nullptr;
		ImageWidget*			healthBack		= nullptr;
		ImageWidget*			healthBar		= nullptr;
		ImageWidget*			hurtOverlay		= nullptr;
		ImageWidget*			progressBack	= nullptr;
		ImageWidget*			progressBar		= nullptr;
		BlackoutRingWidget*		crosshair		= nullptr;
		ImageWidget*			spreadCone		= nullptr;	// red: where the bullets of the local hero can go
		std::vector<TextWidget*>	nameTags;				// over the heads: allies blue, enemies seen red
		ImageWidget*			chargeIcon		= nullptr;	// over the head of the carrier of the charge (his team only)
		std::vector<TextWidget*>	siteMarkers;
		float					hurtTime		= 0.f;
		int						kills			= 0;
		int						state			= 0;		// 0 playing, 1 won, 2 lost (versus: 1 = the round is over)
		float					stateTime		= 0.f;
		bool					started			= false;	// BeginPlay ran: the widgets exist
		bool					leaving			= false;	// a scene is being opened
		bool					escapeHeld		= true;

		// Versus
		bool					versus			= false;
		int						round			= 1;		// shared value "round"
		int						attackers		= 0;
		bool					matchOver		= false;
		class BlackoutBuyZone*	buyZone			= nullptr;		// the circle on the floor of the buy phase (in the world, under the HUD)
		ImageWidget*			blindOverlay	= nullptr;
		float					blindLeft		= 0.f;
		int						localTeam		= 0;
		bool					heroesMade		= false;
		bool					nextRoundSent	= false;
		bool					roundEndSent	= false;
		bool					connectionLost	= false;
		BlackoutChat*			chat			= nullptr;
		std::vector<Event<const NetMessage&>::ListenerHandle>	subscriptions;
		Event<>::ListenerHandle	disconnectHandle;

		// The charge (versus): the host's state, copied by the others (BO_Charge)
		BlackoutCharge*			charge			= nullptr;
		int						chargeState		= Carried;
		int						carrierId		= -1;
		FVector2				chargePosition;
		int						chargeSite		= -1;
		float					roundLeft		= 0.f;
		float					detonateLeft	= 0.f;
		float					syncTimer		= 0.f;
		float					beepTimer		= 0.f;
		float					buyLeft			= 0.f;		// shopping time left (the host's in versus)

		// The shop (B during the buy phase)
		bool					shopOpen		= false;
		bool					shopKeyHeld		= true;
		struct ShopCard
		{
			int						item	= 0;
			class BlackoutButton*	button	= nullptr;
			ImageWidget*			icon	= nullptr;
			TextWidget*				name	= nullptr;
			TextWidget*				price	= nullptr;
		};
		ImageWidget*			shopPanel		= nullptr;
		TextWidget*				shopTitle		= nullptr;
		std::vector<ShopCard>	shopCards;					// a card per item, in columns by kind (like a tactical shooter)
		std::vector<TextWidget*>	shopHeaders;
		TextWidget*				creditsText		= nullptr;
		TextWidget*				moveText		= nullptr;

		ImageWidget*			weaponIcon		= nullptr;	// the gun in hand, over the ammo
		TextWidget*				slotsText		= nullptr;	// "1 Fusil d'assaut   2 Pistolet"
		ImageWidget*			shieldBar		= nullptr;	// over the health bar
		std::map<int, int>		freshWeapon;				// player * 2 + slot -> gun bought in this round (sold back at its full price)

		// Guns on the floor (the host numbers them)
		int						nextDropId		= 1;
		bool					pickupHeld		= true;
		TextWidget*				pickupText		= nullptr;

		// The bar at the top (like a tactical shooter): the scores, the timer, a portrait per player (dark when dead)
		ImageWidget*			topPanel		= nullptr;
		TextWidget*				scoreLeft		= nullptr;	// the local team
		TextWidget*				scoreRight		= nullptr;
		TextWidget*				roundText		= nullptr;
		std::vector<ImageWidget*>	portraits;

		// Pings: a mark the team sees (ping key / middle click), one per player, PING_TIME seconds
		struct PingMark
		{
			int				player	= -1;
			FVector2		position;
			float			age		= 99.f;
			std::string		name;
			ImageWidget*	mark	= nullptr;
			TextWidget*		text	= nullptr;
		};
		static constexpr float	PING_TIME		= 5.f;
		std::vector<PingMark>	pings;
		bool					pingHeld		= true;
		float					pingCooldown	= 0.f;

		int						action			= 0;		// this machine: 0 nothing, 1 planting, 2 defusing
		float					actionTime		= 0.f;
		bool					actionHeard		= false;	// the others were told (an echo)

		/// <returns>Where a world position is on the HUD (1920 x 1080).</returns>
		FVector2 WorldToHud(const FVector2& _position);

		void TickSolo();
		void StartVersus();
		void TickVersus(const float& _deltaTime);
		void LeaveToMenu();
		/// <summary>_side 0: the attackers' corner (south-west), 1: the defenders' one (north-east).</summary>
		static FVector2 SpawnPoint(const int& _side, const int& _index);

		std::vector<BlackoutSite*> GetSites();
		int SiteAt(const FVector2& _position);
		std::string SiteName(const int& _site);
		void UpdateLocalAction(const float& _deltaTime);
		void HostRules(const float& _deltaTime, const int* _alive);
		void SetChargeState(const int& _state, const int& _carrier, const FVector2& _position, const int& _site, const float& _roundLeft, const float& _detonateLeft);
		void BroadcastCharge();
		void HostPlant(const int& _player, const FVector2& _position, const int& _site);
		void HostDefuse(const int& _player);
		void EndRound(const int& _winner, const std::string& _reason);
		void UpdateChargeHud(const float& _deltaTime);

		int Credits(const int& _player) const;
		void BuildShop();
		void SetShopOpen(const bool& _open);
		void UpdateShop();
		void HostBuy(const int& _player, const int& _item);
		void ApplyBought(const int& _player, const int& _item);
		static int ItemPrice(const int& _item);
		void HostRoundRewards(const int& _winner);

		void UpdateEchoes(const float& _deltaTime);
		void UpdateHud(const float& _deltaTime);
		void UpdateSpreadCone();
		/// <summary>The ping key (or the middle click) marks the mouse for the team (BO_Ping); the marks fade after PING_TIME.</summary>
		void UpdatePings(const float& _deltaTime);
		/// <summary>The mark of _player at a world position (his last one moves there).</summary>
		void ShowPing(const int& _player, const FVector2& _position);
		/// <returns>The mouse in the world, through the camera.</returns>
		FVector2 MouseInWorld();
		/// <summary>An enemy nobody of our team sees is not drawn (the dim light would show him behind a wall).</summary>
		void UpdateFog(const float& _deltaTime);
		/// <summary>Shared values of a player's guns and shield: what he starts the next round with.</summary>
		void ResetLoadout(const int& _player);
		void UpdateTopBar();
		/// <summary>Host (or alone): a gun falls at _position, on every machine (BO_Drop).</summary>
		void DropWeapon(const int& _weapon, const FVector2& _position);
		void ApplyDrop(const int& _dropId, const int& _weapon, const FVector2& _position, const float& _angle);
		/// <summary>Host (or alone): _player takes the gun _dropId, his own falls in its place (BO_Picked + BO_Drop).</summary>
		void HostPickup(const int& _player, const int& _dropId);
		void ApplyPicked(const int& _dropId, const int& _player, const int& _weapon);
		/// <summary>E near a gun on the floor: takes it (when E does not plant nor defuse).</summary>
		void UpdatePickup();
		class BlackoutDroppedWeapon* FindDrop(const int& _dropId);
		/// <summary>An image widget showing a gun seen from the side, as big as fits in _box.</summary>
		static void ShowWeaponIcon(ImageWidget* _image, const int& _weapon, const FVector2& _box);
		void UpdateNameTags();
		/// <returns>True when _viewer sees _target: close, or in the beam of his flashlight, with nothing in between.</returns>
		static bool InSight(BlackoutHero& _viewer, BlackoutCharacter& _target);
		/// <summary>Versus: where the charge lies at the start of a round, a little in front of the attackers' corner.</summary>
		FVector2 ChargeStartPoint();
		void SetMessage(const std::string& _text, const Color& _color);
		void SetText(TextWidget* _widget, const std::string& _text, const float& _align);
		TextWidget* MakeText(const FVector2& _position, const unsigned& _size);
	};

}
