#pragma once
#include "BlackoutCharacter.h"

namespace Krampus
{

	/// <summary>
	/// An enemy of Blackout (red soldier). Walks around the arena, shoots the hero once it has seen him for reactionTime, goes to
	/// look where it last saw him or where it heard a shot. Its steps make echoes on the HUD: the hero hears it in the dark.
	/// </summary>
	class BlackoutEnemy : public BlackoutCharacter
	{
		KACTOR(BlackoutEnemy, BlackoutCharacter)

	public:
		void BeginPlay() override;

		BlackoutEnemy(Level* _level);

		KPROPERTY_RANGE(Int, weapon, 0, 10) = 7;					// gun in its hands (only its picture), dropped when it dies: BlackoutWeapon::Get (7 rifle)
		KPROPERTY_RANGE(Float, sightRange, 50.f, 3000.f) = 650.f;	// pixels
		KPROPERTY_RANGE(Float, reactionTime, 0.f, 5.f) = 0.45f;	// seconds before the first shot
		KPROPERTY_RANGE(Float, aimSpread, 0.f, 45.f) = 7.f;		// degrees
		KPROPERTY_RANGE(Float, stepInterval, 0.1f, 3.f) = 0.45f;	// seconds between two echoes while it walks

		/// <summary>A noise at _position (a shot): it goes to look, unless it sees the hero.</summary>
		void Hear(const FVector2& _position);

		void TakeDamage(const float& _amount, const FVector2& _from) override;

		/// <summary>A flash: it sees nothing for _seconds (it stops, does not shoot).</summary>
		void Blind(const float& _seconds) { blindLeft = std::max(blindLeft, _seconds); }

	protected:
		void Tick(const Float& _deltaTime) override;
		void Die() override;

	private:
		FVector2	target;
		bool		hasTarget		= false;	// going to where it saw / heard the hero
		FVector2	wanderPoint;
		bool		hasWanderPoint	= false;
		float		seenTime		= 0.f;		// how long it has seen the hero
		float		stepTime		= 0.f;
		float		stuckTime		= 0.f;
		float		blindLeft		= 0.f;
		FVector2	lastPosition;
		std::vector<FVector2>	path;			// the way to the goal, around the walls (navigation mesh)
		size_t		pathIndex		= 0;
		FVector2	pathGoal;
		float		repathTime		= 0.f;

		/// <summary>Walks toward _goal along a path of the navigation (found again when the goal moves, and twice a second).</summary>
		void MoveTo(const FVector2& _goal, const float& _speed);
		void PickWanderPoint();
	};

}
