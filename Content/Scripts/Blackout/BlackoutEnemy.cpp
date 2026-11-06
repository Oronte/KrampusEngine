#include "BlackoutEnemy.h"
#include "BlackoutHero.h"
#include "BlackoutGameMode.h"
#include "Graphics/Particles/ParticleEmitter.h"
#include "Navigation/NavigationSystem.h"

namespace Krampus
{

BlackoutEnemy::BlackoutEnemy(Level* _level)
	: BlackoutCharacter(_level)
{
	name = "Enemy";
	team = 1;
	maxHealth = 75.f;
	moveSpeed = 170.f;
	bulletDamage = 9.f;
	fireDelay = 0.55f;
	body->GetShapeObject()->GetShape()->SetFillColor(Color(255, 120, 110));
}

void BlackoutEnemy::BeginPlay()
{
	BlackoutCharacter::BeginPlay();
	SetGunLook(static_cast<int>(weapon));
}

void BlackoutEnemy::Hear(const FVector2& _position)
{
	if (seenTime > 0.f) return;
	target = _position;
	hasTarget = true;
}

void BlackoutEnemy::TakeDamage(const float& _amount, const FVector2& _from)
{
	BlackoutCharacter::TakeDamage(_amount, _from);
	if (!IsDead()) Hear(_from);		// shot from the dark: it turns to where it came from
}

void BlackoutEnemy::PickWanderPoint()
{
	BlackoutGameMode* _mode = BlackoutGameMode::Of(GetLevel());
	if (!_mode) return;

	// Somewhere else in the arena, on the navigation when there is one.
	FVector2 _point = _mode->RandomPoint(100.f);
	FVector2 _onMesh;
	if (GetLevel()->GetNavigation().Project(_point, 200.f, _onMesh, 22.f)) _point = _onMesh;
	wanderPoint = _point;
	hasWanderPoint = true;
}

void BlackoutEnemy::MoveTo(const FVector2& _goal, const float& _speed)
{
	repathTime -= static_cast<float>(GetDeltaTime());
	if (path.empty() || pathIndex >= path.size() || repathTime <= 0.f || (_goal - pathGoal).Length() > 60.f)
	{
		path.clear();
		const NavPath _found = GetLevel()->GetNavigation().FindPath(GetPosition(), _goal, 22.f);
		if (_found.found)
			for (const NavPathPoint& _point : _found.points) path.push_back(_point.position);
		if (path.empty()) path.push_back(_goal);		// no navigation: straight to it
		pathIndex = 0;
		pathGoal = _goal;
		repathTime = 0.5f;
	}

	// The next point of the path that is not reached yet.
	while (pathIndex + 1 < path.size() && (path[pathIndex] - GetPosition()).Length() < 24.f) ++pathIndex;
	const FVector2 _offset = path[pathIndex] - GetPosition();
	if (_offset.Length() < 2.f) { Walk(FVector2(), 0.f); return; }
	Walk(_offset.Normalized(), _speed);
	LookAt(path[pathIndex]);
}

void BlackoutEnemy::Tick(const Float& _deltaTime)
{
	BlackoutCharacter::Tick(_deltaTime);
	if (IsDead()) return;

	const float _delta = static_cast<float>(_deltaTime);
	BlackoutGameMode* _mode = BlackoutGameMode::Of(GetLevel());
	BlackoutHero* _hero = _mode ? _mode->GetHero() : nullptr;
	const float _speed = static_cast<float>(moveSpeed);

	// The hero is shopping: the enemies wait. Blinded by a flash: it stands, sees nothing.
	if (blindLeft > 0.f) blindLeft -= _delta;
	if ((_mode && _mode->IsBuyPhase()) || blindLeft > 0.f)
	{
		Walk(FVector2(), 0.f);
		seenTime = 0.f;
		return;
	}

	if (_hero && !_hero->IsDead() && CanSee(_hero, static_cast<float>(sightRange)))
	{
		// Seen: it turns to him, keeps some distance, and shoots after its reaction time.
		seenTime += _delta;
		target = _hero->GetPosition();
		hasTarget = true;
		AimAt(target);		// the gun at him, not the head

		const float _distance = (target - GetPosition()).Length();
		if (_distance > 380.f) Walk((target - GetPosition()).Normalized(), _speed * 0.6f);
		else Walk(FVector2(), 0.f);

		if (seenTime >= static_cast<float>(reactionTime)) Fire(target, static_cast<float>(aimSpread));
	}
	else
	{
		seenTime = 0.f;
		if (hasTarget)
		{
			// To where it last saw or heard him, faster.
			MoveTo(target, _speed * 1.15f);
			if ((target - GetPosition()).Length() < 40.f) hasTarget = false;
		}
		else
		{
			if (!hasWanderPoint) PickWanderPoint();
			MoveTo(wanderPoint, _speed);
			if ((wanderPoint - GetPosition()).Length() < 40.f) hasWanderPoint = false;
		}

		// Blocked by a wall or another body: somewhere else.
		if ((GetPosition() - lastPosition).Length() < _speed * _delta * 0.2f) stuckTime += _delta;
		else stuckTime = 0.f;
		if (stuckTime > 0.8f)
		{
			stuckTime = 0.f;
			hasTarget = false;
			hasWanderPoint = false;
			path.clear();
		}
	}

	// Its steps are heard: an echo every stepInterval while it walks.
	const bool _walking = (GetPosition() - lastPosition).Length() > _speed * _delta * 0.3f;
	lastPosition = GetPosition();
	stepTime -= _delta;
	if (_walking && stepTime <= 0.f && _mode)
	{
		stepTime = static_cast<float>(stepInterval);
		_mode->Echo(GetPosition(), Color(255, 70, 70), 34.f);
	}
}

void BlackoutEnemy::Die()
{
	GetLevel()->SpawnParticles(ParticlePreset::Blood, GetPosition());
	if (BlackoutGameMode* _mode = BlackoutGameMode::Of(GetLevel()))
		_mode->EnemyKilled(GetPosition(), GetGunLook());
	Destroy();
}

}
