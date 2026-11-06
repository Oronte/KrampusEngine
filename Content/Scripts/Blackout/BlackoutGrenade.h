#pragma once
#include "BlackoutCharacter.h"

namespace Krampus
{

	/// <summary>
	/// A grenade of Blackout, thrown by the hero (right click or G). It rolls toward the aimed point, slows down, bounces on the walls and
	/// blinks, then explodes after its fuse: everyone in the radius that the walls do not shield is hurt (the closer, the more, the
	/// thrower too), a burn marks the floor and the walls, a flash lights the place and the enemies hear it.
	/// A flash (F) rolls the same way but hurts nobody: it blinds whoever sees it go off (BlindAmount: close, nothing in between,
	/// looking at it), the white screen of BlackoutGameMode::Blind for the player, BlackoutEnemy::Blind for the solo enemies.
	/// A smoke (C) rolls the same way, then leaves a BlackoutSmoke cloud.
	/// Made by BlackoutHero, never saved in a scene.
	/// </summary>
	class BlackoutGrenade : public Actor
	{
	public:
		enum Kind { Frag = 0, Flash = 1, Smoke = 2 };		// sent as an int in BO_Grenade

		static constexpr float	FUSE		= 1.6f;		// seconds
		static constexpr float	SMOKE_FUSE	= 1.2f;		// seconds
		static constexpr float	RADIUS		= 230.f;	// pixels
		static constexpr float	DAMAGE		= 110.f;	// at the center, a quarter of it at the edge
		static constexpr float	MAX_RANGE	= 700.f;	// pixels it can be thrown
		static constexpr float	FLASH_FUSE	= 1.f;		// seconds
		static constexpr float	FLASH_RADIUS	= 700.f;	// pixels: farther, nobody is blinded
		static constexpr float	BLIND_TIME	= 3.2f;		// seconds of blindness for a flash right in the eyes

		BlackoutGrenade(Level* _level);

		/// <summary>
		/// Leaves from where it is toward _target (it stops about there). _real false: the copy of a grenade thrown on another machine,
		/// only seen (that machine tells the hits). In a network game it does not hurt the thrower's teammates. _kind: Kind.
		/// </summary>
		void Throw(const BlackoutCharacter& _thrower, const FVector2& _target, const bool& _real = true, const int& _kind = Frag);

		/// <returns>0..1: how much the flash, going off here now, blinds _target.</returns>
		float BlindAmount(const Actor& _target);

	protected:
		void Tick(const Float& _deltaTime) override;

	private:
		SpriteComponent*		ball		= nullptr;
		PointLightComponent*	light		= nullptr;
		FVector2				velocity;
		float					fuse		= FUSE;
		float					flash		= -1.f;		// >= 0 once exploded: seconds the light burst still lasts
		std::uint64_t			throwerId	= 0;
		int						throwerTeam	= 0;
		int						throwerPlayer	= -1;
		bool					real		= true;
		bool					blinding	= false;	// a flash
		bool					smoke		= false;	// a smoke

		void Roll(const float& _deltaTime);
		void Explode();
		void Burst();
	};

	/// <summary>
	/// The cloud of a smoke: a grey disc that grows, lasts DURATION seconds, then thins out. It stops the light (ShadowCasterComponent:
	/// the flashlights do not go through) and the sight (Blocks: names over the heads, solo enemies, flashes). Bullets go through.
	/// Made by BlackoutGrenade on every machine (the copies too), never saved.
	/// </summary>
	class BlackoutSmoke : public Actor
	{
	public:
		static constexpr float	RADIUS		= 170.f;	// pixels, once grown
		static constexpr float	DURATION	= 12.f;		// seconds
		static constexpr float	GROW		= 0.8f;		// seconds to reach its size
		static constexpr float	FADE		= 1.5f;		// last seconds: it shrinks and fades
		static constexpr float	NEAR_SIGHT	= 90.f;		// pixels: closer than this, two soldiers see each other even in the smoke

		BlackoutSmoke(Level* _level);

		/// <returns>True when a cloud of the level is in the way between _from and _to (or one of them is inside it).</returns>
		static bool Blocks(Level* _level, const FVector2& _from, const FVector2& _to);

	protected:
		void Tick(const Float& _deltaTime) override;

	private:
		SpriteComponent*		cloud		= nullptr;
		ShadowCasterComponent*	shadow		= nullptr;
		float					age			= 0.f;
		float					radius		= 20.f;
		float					puffTime	= 0.f;
	};

}
