#include "BlackoutGrenade.h"
#include "BlackoutGameMode.h"
#include "BlackoutHero.h"
#include "BlackoutEnemy.h"
#include "Components/StainComponent.h"
#include "Graphics/Particles/ParticleEmitter.h"

namespace Krampus
{

namespace
{
	const float		FRICTION	= 2.2f;		// speed lost per second (the distance rolled is speed / FRICTION)
	const float		BOUNCE		= 0.5f;		// speed kept after a wall
	const float		FLASH_TIME	= 0.35f;
}

BlackoutGrenade::BlackoutGrenade(Level* _level)
	: Actor(_level)
{
	name = "BlackoutGrenade";

	ball = CreateComponent<SpriteComponent>(CircleShapeData(7.f, "Blackout/Solid"));
	ball->GetShapeObject()->GetShape()->SetFillColor(Color(70, 95, 60));
	ball->GetShapeObject()->GetShape()->SetOutlineThickness(2.f);
	ball->GetShapeObject()->GetShape()->SetOutlineColor(Color(20, 25, 20));
	ball->SetZOrder(ZOrder::Projectiles);

	// A small red blink: the grenade can be seen in the dark, and it blinks faster before it explodes.
	light = CreateComponent<PointLightComponent>();
	light->color = Color(255, 60, 40);
	light->radius = 60.f;
	light->castShadows = false;
}

void BlackoutGrenade::Throw(const BlackoutCharacter& _thrower, const FVector2& _target, const bool& _real, const int& _kind)
{
	throwerId = _thrower.GetId();
	throwerTeam = _thrower.GetTeam();
	throwerPlayer = _thrower.GetPlayerId();
	real = _real;
	blinding = _kind == Flash;
	smoke = _kind == Smoke;
	if (blinding)
	{
		// A flash: white, a white blink, a shorter fuse.
		fuse = FLASH_FUSE;
		ball->GetShapeObject()->GetShape()->SetFillColor(Color(215, 220, 230));
		light->color = Color(220, 230, 255);
	}
	else if (smoke)
	{
		// A smoke: grey, a grey blink.
		fuse = SMOKE_FUSE;
		ball->GetShapeObject()->GetShape()->SetFillColor(Color(120, 125, 130));
		light->color = Color(170, 175, 185);
	}
	const FVector2 _offset = _target - GetPosition();
	const float _distance = std::min(_offset.Length(), MAX_RANGE);
	const FVector2 _direction = _offset.Length() > 1.f ? _offset.Normalized() : _thrower.GetRightVector();
	velocity = _direction * (_distance * FRICTION);
}

float BlackoutGrenade::BlindAmount(const Actor& _target)
{
	// 0..1: how much a flash blinds _target: close, nothing in between, looking at it (from the side or the back: less).
	const FVector2 _center = GetPosition();
	const float _distance = (_target.GetPosition() - _center).Length();
	if (_distance > FLASH_RADIUS) return 0.f;
	const RaycastHitInfo _hit = GetLevel()->GetWorldPhysics().Raycast(_center, _target.GetPosition());
	if (_hit.hit && _hit.actor && _hit.actor != &_target && !Cast<BlackoutCharacter>(_hit.actor)) return 0.f;
	if (BlackoutSmoke::Blocks(GetLevel(), _center, _target.GetPosition())) return 0.f;		// a smoke in between

	const FVector2 _toFlash = _distance > 1.f ? (_center - _target.GetPosition()) / _distance : _target.GetRightVector();
	const FVector2 _look = _target.GetRightVector();
	const float _facing = _look.x * _toFlash.x + _look.y * _toFlash.y;		// 1: right at it, -1: back turned
	const float _eyes = 0.25f + 0.75f * std::clamp((_facing + 0.2f) / 1.2f, 0.f, 1.f);
	return std::clamp((1.f - _distance / FLASH_RADIUS) * 1.4f, 0.f, 1.f) * _eyes;
}

void BlackoutGrenade::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);
	const float _delta = static_cast<float>(_deltaTime);

	// Exploded: only the flash is left.
	if (flash >= 0.f)
	{
		flash -= _delta;
		light->intensity = std::max(0.f, (blinding ? 5.f : 3.f) * flash / FLASH_TIME);
		if (flash <= 0.f) Destroy();
		return;
	}

	Roll(_delta);

	fuse -= _delta;
	const float _period = fuse > 0.6f ? 0.35f : 0.12f;
	light->intensity = std::fmod(fuse, _period) < _period * 0.4f ? 1.2f : 0.15f;
	if (fuse <= 0.f)
	{
		if (smoke)
		{
			// The cloud; every machine makes its own (a copy too): the light and the sight stop the same everywhere.
			BlackoutSmoke* _cloud = GetLevel()->SpawnActor<BlackoutSmoke>();
			_cloud->SetPosition(GetPosition());
			if (BlackoutGameMode* _mode = BlackoutGameMode::Of(GetLevel())) _mode->Echo(GetPosition(), Color(190, 195, 205), 120.f);
			LOG(VerbosityType::Log, "Blackout: smoke");
			Destroy();
		}
		else if (blinding) Burst();
		else Explode();
	}
}

void BlackoutGrenade::Roll(const float& _deltaTime)
{
	const FVector2 _from = GetPosition();
	const FVector2 _to = _from + velocity * _deltaTime;
	if ((_to - _from).Length() < 0.01f) return;

	// The first body on the way (not the thrower, just after the throw): it bounces off it.
	for (const RaycastHitInfo& _hit : GetLevel()->GetWorldPhysics().RaycastAll(_from, _to))
	{
		if (!_hit.hit || !_hit.actor || _hit.actor->IsPendingDestroy() || _hit.actor->GetId() == throwerId) continue;

		const FVector2 _direction = velocity.Normalized();
		FVector2 _normal = _hit.normal.Length() > 0.f ? _hit.normal.Normalized() : _direction * -1.f;
		const float _along = velocity.x * _normal.x + velocity.y * _normal.y;
		velocity = (velocity - _normal * (2.f * _along)) * BOUNCE;
		SetPosition(_hit.point - _direction * 6.f);		// a little before the wall
		return;
	}
	SetPosition(_to);
	velocity = velocity * std::exp(-FRICTION * _deltaTime);
}

void BlackoutGrenade::Explode()
{
	const FVector2 _center = GetPosition();
	Level* _level = GetLevel();

	BlackoutEffect(_level, ParticlePreset::Explosion, _center);
	BlackoutEffect(_level, ParticlePreset::Smoke, _center);
	StainComponent::StainAt(_level, _center, 75.f, Color(15, 12, 10, 215), StainStyle::Scorch);

	// Everyone close enough that no wall shields (another character does not). Only the thrower's machine tells the hits.
	BlackoutGameMode* _mode = BlackoutGameMode::Of(_level);
	const bool _versus = _mode && _mode->IsVersus();
	for (BlackoutCharacter* _character : real ? _level->GetActorManager().GetAllActorOfClass<BlackoutCharacter>() : std::vector<BlackoutCharacter*>())
	{
		if (_character->IsDead() || _character->IsPendingDestroy()) continue;
		if (_versus && _character->GetTeam() == throwerTeam && _character->GetId() != throwerId) continue;	// no harm to teammates
		const float _distance = (_character->GetPosition() - _center).Length();
		if (_distance > RADIUS) continue;

		const RaycastHitInfo _hit = _level->GetWorldPhysics().Raycast(_center, _character->GetPosition());
		if (_hit.hit && _hit.actor && !Cast<BlackoutCharacter>(_hit.actor)) continue;

		const float _damage = DAMAGE * (1.f - 0.75f * _distance / RADIUS);
		if (_versus) _mode->SendHit(_character->GetPlayerId(), _damage, _center, throwerPlayer);
		else _character->TakeDamage(_damage, _center);
	}

	if (_mode) _mode->Explosion(_center);

	// The flash: a big orange light that fades, the grenade itself is gone.
	ball->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 0));
	ball->GetShapeObject()->GetShape()->SetOutlineColor(Color(0, 0, 0, 0));
	light->color = Color(255, 170, 90);
	light->radius = RADIUS * 2.f;
	light->castShadows = true;
	flash = FLASH_TIME;
	LOG(VerbosityType::Log, "Blackout: grenade exploded");
}

void BlackoutGrenade::Burst()
{
	// No damage: it blinds. Every machine blinds its own player (and the enemies of the solo game): a copy blinds as well as the real one.
	const FVector2 _center = GetPosition();
	Level* _level = GetLevel();
	BlackoutEffect(_level, ParticlePreset::Sparks, _center);

	if (BlackoutGameMode* _mode = BlackoutGameMode::Of(_level))
	{
		if (BlackoutHero* _hero = _mode->GetHero())
			if (!_hero->IsDead())
				if (const float _amount = BlindAmount(*_hero); _amount > 0.f) _mode->Blind(0.3f + BLIND_TIME * _amount);
		for (BlackoutEnemy* _enemy : _level->GetActorManager().GetAllActorOfClass<BlackoutEnemy>())
			if (!_enemy->IsDead())
				if (const float _amount = BlindAmount(*_enemy); _amount > 0.f) _enemy->Blind(0.3f + BLIND_TIME * _amount);
		_mode->Echo(_center, Color(235, 240, 255), 200.f);
	}

	ball->GetShapeObject()->GetShape()->SetFillColor(Color(0, 0, 0, 0));
	ball->GetShapeObject()->GetShape()->SetOutlineColor(Color(0, 0, 0, 0));
	light->color = Color(235, 240, 255);
	light->radius = FLASH_RADIUS * 1.3f;
	light->castShadows = true;
	flash = FLASH_TIME;
	LOG(VerbosityType::Log, "Blackout: flash");
}

// ---- BlackoutSmoke

BlackoutSmoke::BlackoutSmoke(Level* _level)
	: Actor(_level)
{
	name = "BlackoutSmoke";

	// Over the floor and the walls, under the soldiers: the enemies inside are hidden by the fog of BlackoutGameMode (Blocks), the
	// soldiers very close to us stay seen in it.
	cloud = CreateComponent<SpriteComponent>(CircleShapeData(radius, "Blackout/Solid"));
	cloud->GetShapeObject()->GetShape()->SetFillColor(Color(105, 110, 118, 0));
	cloud->GetShapeObject()->GetShape()->SetOutlineThickness(14.f);
	cloud->GetShapeObject()->GetShape()->SetOutlineColor(Color(105, 110, 118, 0));
	cloud->SetZOrder(static_cast<std::uint8_t>(static_cast<int>(ZOrder::Actors) - 2));		// over the guns on the floor (Actors - 3)

	// The lights stop on it (the shadow follows the disc of the sprite).
	shadow = CreateComponent<ShadowCasterComponent>();
}

bool BlackoutSmoke::Blocks(Level* _level, const FVector2& _from, const FVector2& _to)
{
	if (!_level) return false;
	const FVector2 _segment = _to - _from;
	const float _length2 = _segment.x * _segment.x + _segment.y * _segment.y;
	if (_length2 < NEAR_SIGHT * NEAR_SIGHT) return false;		// very close: seen even in the smoke
	for (BlackoutSmoke* _smoke : _level->GetActorManager().GetAllActorOfClass<BlackoutSmoke>())
	{
		const float _radius = _smoke->radius * 0.85f;
		if (_smoke->IsPendingDestroy() || _radius < 25.f) continue;

		// The point of the segment closest to the center of the cloud.
		const FVector2 _toCenter = _smoke->GetPosition() - _from;
		const float _along = _length2 > 0.f ? std::clamp((_toCenter.x * _segment.x + _toCenter.y * _segment.y) / _length2, 0.f, 1.f) : 0.f;
		const FVector2 _closest = _from + _segment * _along;
		if ((_smoke->GetPosition() - _closest).Length() < _radius) return true;
	}
	return false;
}

void BlackoutSmoke::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);
	const float _delta = static_cast<float>(_deltaTime);
	age += _delta;
	if (age >= DURATION)
	{
		Destroy();
		return;
	}

	// It grows fast, stays, then shrinks and fades during the last FADE seconds.
	const float _grow = std::clamp(age / GROW, 0.f, 1.f);
	const float _fade = std::clamp((DURATION - age) / FADE, 0.f, 1.f);
	radius = RADIUS * (0.2f + 0.8f * (1.f - (1.f - _grow) * (1.f - _grow))) * (0.4f + 0.6f * _fade);
	cloud->SetRadius(radius);
	const std::uint8_t _alpha = static_cast<std::uint8_t>(240.f * _fade);
	cloud->GetShapeObject()->GetShape()->SetFillColor(Color(105, 110, 118, _alpha));
	cloud->GetShapeObject()->GetShape()->SetOutlineColor(Color(105, 110, 118, static_cast<std::uint8_t>(_alpha / 2)));

	// Puffs that drift inside: it looks alive.
	puffTime -= _delta;
	if (puffTime <= 0.f && _fade > 0.5f)
	{
		puffTime = 0.45f;
		const float _turn = BlackoutRandom(0.f, 6.2831853f);
		BlackoutEffect(GetLevel(), ParticlePreset::Smoke, GetPosition() + FVector2(std::cos(_turn), std::sin(_turn)) * BlackoutRandom(0.f, radius * 0.6f));
	}
}

}
