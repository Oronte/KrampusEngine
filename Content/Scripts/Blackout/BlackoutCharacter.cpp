#include "BlackoutCharacter.h"
#include "BlackoutGameMode.h"
#include "BlackoutGrenade.h"
#include "Core/Engine.h"
#include "Graphics/Particles/ParticleEmitter.h"
#include "Components/ParticleComponent.h"
#include "Components/StainComponent.h"
#include <random>

namespace Krampus
{

namespace
{
	const FVector2	SPRITE_SIZE		= FVector2(96.f, 72.f);		// Player.png (577 x 432), facing right
	const FVector2	SPRITE_PIVOT	= FVector2(36.f, 37.5f);	// its head, at this size
	const float		BODY_RADIUS		= 22.f;
	const float		GUN_SIDE		= 15.f;						// the hands are on the right of the head (+y, facing +x)
}

const BlackoutWeapon& BlackoutWeapon::Get(const int& _index)
{
	static const BlackoutWeapon WEAPONS[COUNT] =
	{
		//	name					damage	delay	spread	pellets	mag	reload	speed	range	auto	view	price	texture			icon				size in the hands		grip	category	tint
		// Sidearms (slot 2)
		{ "Pistolet",				18.f,	0.25f,	2.f,	1,		12,	1.5f,	1500.f,	1000.f,	false,	1440.f,	0,		"Pistol",		"PistolSide",		FVector2(30.f, 7.f),	42.f,	Sidearm,	Color::White() },
		{ "Pistolet auto",			13.f,	0.09f,	3.5f,	1,		18,	1.6f,	1500.f,	850.f,	true,	1440.f,	400,	"Pistol",		"PistolSide",		FVector2(28.f, 6.6f),	43.f,	Sidearm,	Color(150, 190, 255) },
		{ "Pistolet lourd",			42.f,	0.55f,	1.f,	1,		6,	2.f,	1800.f,	1300.f,	false,	1440.f,	800,	"Pistol",		"PistolSide",		FVector2(36.f, 8.4f),	40.f,	Sidearm,	Color(235, 190, 110) },
		{ "Mini pompe",				9.f,	0.6f,	15.f,	6,		2,	1.8f,	1200.f,	300.f,	false,	1440.f,	300,	"Shotgun",		"ShotgunSide",		FVector2(44.f, 5.f),	30.f,	Sidearm,	Color(215, 165, 130) },
		// Shotguns
		{ "Fusil a pompe",			14.f,	0.85f,	13.f,	8,		6,	2.4f,	1300.f,	430.f,	false,	1440.f,	1600,	"Shotgun",		"ShotgunSide",		FVector2(82.f, 6.6f),	6.f,	Shotgun,	Color::White() },
		{ "Pompe automatique",		11.f,	0.3f,	11.f,	7,		7,	2.6f,	1300.f,	400.f,	false,	1440.f,	2100,	"Shotgun",		"ShotgunSide",		FVector2(76.f, 7.5f),	8.f,	Shotgun,	Color(150, 200, 150) },
		// Rifles
		{ "Carabine",				19.f,	0.1f,	3.8f,	1,		30,	2.f,	1550.f,	1000.f,	true,	1440.f,	2050,	"AssaultRifle",	"AssaultRifleSide",	FVector2(68.f, 8.f),	10.f,	Rifle,		Color(190, 215, 160) },
		{ "Fusil d'assaut",			22.f,	0.11f,	3.f,	1,		25,	2.f,	1600.f,	1150.f,	true,	1440.f,	2900,	"AssaultRifle",	"AssaultRifleSide",	FVector2(78.f, 8.7f),	8.f,	Rifle,		Color::White() },
		{ "Fusil tactique",			48.f,	0.32f,	0.9f,	1,		12,	2.4f,	2200.f,	1500.f,	false,	1600.f,	2400,	"AssaultRifle",	"AssaultRifleSide",	FVector2(82.f, 9.f),	6.f,	Rifle,		Color(200, 175, 235) },
		// Snipers
		{ "Sniper leger",			85.f,	1.1f,	0.5f,	1,		5,	2.4f,	2800.f,	2200.f,	false,	1750.f,	1100,	"Sniper",		"SniperSide",		FVector2(84.f, 7.6f),	6.f,	Sniper,		Color(185, 225, 185) },
		{ "Sniper",					120.f,	1.4f,	0.3f,	1,		5,	2.8f,	3200.f,	2600.f,	false,	1900.f,	4700,	"Sniper",		"SniperSide",		FVector2(92.f, 8.4f),	4.f,	Sniper,		Color::White() },
	};
	return WEAPONS[std::clamp(_index, 0, COUNT - 1)];
}

std::string BlackoutWeapon::CategoryName(const int& _category)
{
	switch (_category)
	{
	case Sidearm:	return "PISTOLETS";
	case Shotgun:	return "POMPES";
	case Rifle:		return "FUSILS";
	default:		return "SNIPERS";
	}
}

float BlackoutRandom(const float& _min, const float& _max)
{
	static std::mt19937 _random{ std::random_device{}() };
	return std::uniform_real_distribution<float>(_min, std::max(_min, _max))(_random);
}

ParticleComponent* BlackoutEffect(Level* _level, const ParticlePreset& _preset, const FVector2& _position)
{
	// The Sparks and Smoke presets loop: here they play once, and their actor goes away when the last particle is gone.
	ParticleComponent* _effect = _level->SpawnParticles(_preset, _position);
	_effect->Settings().looping = false;
	_effect->destroyOwnerWhenDone = true;
	return _effect;
}

void BlackoutBleed(Level* _level,const Actor* _victim, const FVector2& _position, const FVector2& _direction, const float& _amount)
{
	const Color _blood(115, 0, 0, 225);

	// Drops thrown along the shot: they stop on the floor (no gravity, seen from above) and mark what is under them.
	ParticleComponent* _drops = _level->SpawnParticles(ParticlePreset::Blood, _position);
	ParticleSettings& _settings = _drops->Settings();
	_settings.gravity = FVector2();
	_settings.direction = std::atan2(_direction.y, _direction.x) * 180.f / 3.14159265f;
	_settings.spread = 70.f;
	_settings.speedMin = 60.f;
	_settings.speedMax = 260.f;
	_settings.drag = 3.5f;
	_settings.lifetimeMin = 0.35f;
	_settings.lifetimeMax = 0.7f;
	_settings.burst = static_cast<int>(18.f * _amount);
	_settings.stainMode = static_cast<int>(ParticleStainMode::OnDeath);
	_settings.stainColor = _blood;
	_settings.stainSize = 5.f;
	_settings.stainChance = 0.7f;

	// A splash on the floor behind the victim.
	StainComponent::StainAt(_level, _position + _direction * BlackoutRandom(14.f, 40.f), BlackoutRandom(14.f, 22.f) * std::sqrt(_amount), _blood, StainStyle::Splat, _victim);

	// The wall behind, if it is close: sprayed where the shot would have gone on.
	const RaycastHitInfo _hit = _level->GetWorldPhysics().Raycast(_position, _position + _direction * 170.f, WorldPhysics::AllChannels(), _victim);
	if (_hit.hit && _hit.actor)
		if (StainComponent* _wall = _hit.actor->GetComponent<StainComponent>())
			_wall->Stain(_hit.point + _direction * 6.f, BlackoutRandom(14.f, 24.f) * std::sqrt(_amount), _blood, StainStyle::Splat);
}

// ---- BlackoutCharacter

BlackoutCharacter::BlackoutCharacter(Level* _level)
	: Actor(_level)
{
	name = "BlackoutCharacter";

	body = CreateComponent<SpriteComponent>(RectangleShapeData(SPRITE_SIZE, "Player"));
	body->SetZOrder(ZOrder::Actors);
	PlaceSprite();

	physics = CreateComponent<PhysicsComponent>(Float(BODY_RADIUS));
	physics->SetDynamic();
	physics->SetGravityScale(0.f);
	physics->SetFixedRotation(true);
	physics->EnableSleeping(false);
}

void BlackoutCharacter::PlaceSprite()
{
	body->GetShapeObject()->GetShape()->SetOrigin(SPRITE_PIVOT);
}

void BlackoutCharacter::BeginPlay()
{
	Actor::BeginPlay();
	// A scene keeps the size and the part shown of an older picture: the whole picture again, at its size.
	ShapeObject* _picture = body->GetShapeObject();
	const UVector2 _pixels = _picture->GetTextureRef().GetSize();
	_picture->GetShape()->SetTextureRect(IRect(IVector2(0, 0), IVector2(static_cast<int>(_pixels.x), static_cast<int>(_pixels.y))));
	_picture->SetSize(SPRITE_SIZE);
	PlaceSprite();		// the scene set the size again after the constructor, which centers the picture
	tint = body->GetShapeObject()->GetShape()->GetFillColor();
	health = static_cast<float>(maxHealth);
}

void BlackoutCharacter::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);
	const float _delta = static_cast<float>(_deltaTime);
	fireCooldown = std::max(0.f, fireCooldown - _delta);

	if (hurtFlash > 0.f)
	{
		hurtFlash -= _delta;
		Repaint();
	}
}

void BlackoutCharacter::Repaint()
{
	// The body (red for a moment when hit) and the gun, as much as he is shown.
	const auto _faded = [this](Color _color) { _color.a = static_cast<std::uint8_t>(static_cast<float>(_color.a) * shown); return _color; };
	body->GetShapeObject()->GetShape()->SetFillColor(_faded(hurtFlash > 0.f ? Color(255, 90, 90) : tint));
	if (gun) gun->GetShapeObject()->GetShape()->SetFillColor(gunHidden ? Color(0, 0, 0, 0) : _faded(BlackoutWeapon::Get(gunLook).tint));
}

void BlackoutCharacter::SetShown(const float& _shown)
{
	const float _value = std::clamp(_shown, 0.f, 1.f);
	if (std::abs(_value - shown) < 0.004f) return;
	shown = _value;
	Repaint();
}

void BlackoutCharacter::AimAt(const FVector2& _target)
{
	// The gun is GUN_SIDE on the right of the head: turn a little more so that its line goes through the target.
	const FVector2 _offset = _target - GetPosition();
	const float _distance = _offset.Length();
	if (_distance < 1.f) return;
	float _angle = std::atan2(_offset.y, _offset.x);
	if (_distance > GUN_SIDE + 1.f) _angle -= std::asin(GUN_SIDE / _distance);
	SetRotation(Angle::FromDegrees(_angle * 180.f / 3.14159265f));
}

FVector2 BlackoutCharacter::AimDirection(const FVector2& _target)
{
	// The target must be in front of the muzzle; closer (between the head and the gun, or behind): straight ahead.
	const FVector2 _forward = GetRightVector();
	const FVector2 _toTarget = _target - MuzzlePosition();
	if (_toTarget.x * _forward.x + _toTarget.y * _forward.y < 20.f) return _forward;
	return _toTarget.Normalized();
}

void BlackoutCharacter::Walk(const FVector2& _direction, const float& _speed)
{
	physics->SetLinearVelocity(FVector2(_direction.x * _speed, _direction.y * _speed));
}

void BlackoutCharacter::SetGunLook(const int& _weapon)
{
	gunLook = std::clamp(_weapon, 0, BlackoutWeapon::COUNT - 1);
	const BlackoutWeapon& _stats = BlackoutWeapon::Get(gunLook);
	if (!gun)
	{
		gun = CreateComponent<BlackoutGunComponent>(RectangleShapeData(_stats.size, _stats.texture));
		gun->SetZOrder(static_cast<std::uint8_t>(static_cast<int>(ZOrder::Actors) + 1));		// over the soldier
	}
	else
	{
		gun->GetShapeObject()->SetTexture(_stats.texture);
		gun->GetShapeObject()->SetSize(_stats.size);
	}
	// The back of the gun at (grip, GUN_SIDE) from the head: the origin is the head, seen from the picture.
	gun->GetShapeObject()->GetShape()->SetOrigin(FVector2(-_stats.grip, _stats.size.y / 2.f - GUN_SIDE));
	gunHidden = false;
	Repaint();
}

void BlackoutCharacter::HideGun()
{
	gunHidden = true;
	Repaint();
}

FVector2 BlackoutCharacter::MuzzlePosition()
{
	const FVector2 _forward = GetRightVector();
	const FVector2 _side(-_forward.y, _forward.x);
	const BlackoutWeapon& _stats = BlackoutWeapon::Get(gunLook);
	const FVector2 _muzzle = GetPosition() + _forward * (_stats.grip + _stats.size.x) + _side * GUN_SIDE;
	// Gun inside a wall: the bullet starts from the head, so that the wall stops it.
	if (GetLevel()->GetWorldPhysics().Raycast(GetPosition(), _muzzle, WorldPhysics::AllChannels(), this).hit) return GetPosition();
	return _muzzle;
}

bool BlackoutCharacter::Fire(const FVector2& _target, const float& _spread)
{
	if (fireCooldown > 0.f || IsDead()) return false;
	fireCooldown = static_cast<float>(fireDelay);

	const FVector2 _muzzle = MuzzlePosition();

	// From the muzzle to the target, each pellet turned a little (aiming is never perfect, a shotgun spreads).
	const FVector2 _aim = AimDirection(_target);
	lastMuzzle = _muzzle;
	lastDirections.clear();
	for (int _pellet = 0; _pellet < std::max(1, pellets); ++_pellet)
	{
		const float _turn = BlackoutRandom(-_spread, _spread) * 3.14159265f / 180.f;
		const FVector2 _direction(_aim.x * std::cos(_turn) - _aim.y * std::sin(_turn), _aim.x * std::sin(_turn) + _aim.y * std::cos(_turn));

		BlackoutBullet* _bullet = GetLevel()->SpawnActor<BlackoutBullet>();
		_bullet->SetPosition(_muzzle);
		_bullet->Launch(*this, _direction, static_cast<float>(bulletDamage), true, bulletSpeed, bulletRange);
		lastDirections.push_back(_direction);
	}

	if (BlackoutGameMode* _mode = BlackoutGameMode::Of(GetLevel()))
		_mode->Shot(*this, _muzzle);
	return true;
}

void BlackoutCharacter::TakeDamage(const float& _amount, const FVector2& _from)
{
	if (IsDead()) return;
	// The shield takes the damage first.
	const float _absorbed = std::min(shield, _amount);
	shield -= _absorbed;
	health = std::max(0.f, health - (_amount - _absorbed));
	hurtFlash = 0.12f;
	Repaint();

	const FVector2 _away = (GetPosition() - _from).Length() > 1.f ? (GetPosition() - _from).Normalized() : GetRightVector() * -1.f;
	BlackoutBleed(GetLevel(), this, GetPosition(), _away, IsDead() ? 2.5f : 1.f);
	if (IsDead())
	{
		// A pool where the body falls.
		StainComponent::StainAt(GetLevel(), GetPosition(), 34.f, Color(95, 0, 0, 235), StainStyle::Splat, this);
		physics->SetLinearVelocity(FVector2());
		HideGun();		// it falls on the floor (BlackoutGameMode drops a BlackoutDroppedWeapon)
		Die();
	}
}

bool BlackoutCharacter::CanSee(const Actor* _other, const float& _range)
{
	if (!_other) return false;
	const FVector2 _from = GetPosition();
	const FVector2 _to = _other->GetPosition();
	if ((_to - _from).Length() > _range) return false;

	if (BlackoutSmoke::Blocks(GetLevel(), _from, _to)) return false;		// a smoke in between
	const RaycastHitInfo _hit = GetLevel()->GetWorldPhysics().Raycast(_from, _to, WorldPhysics::AllChannels(), this);
	return !_hit.hit || _hit.actor == _other;
}

// ---- BlackoutBullet

BlackoutBullet::BlackoutBullet(Level* _level)
	: Actor(_level)
{
	name = "BlackoutBullet";

	line = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(22.f, 3.f), "Blackout/Solid"));
	line->SetZOrder(ZOrder::Projectiles);

	// It lights a little around it as it flies (the walls stop that light): what it passes near is seen.
	glow = CreateComponent<PointLightComponent>();
	glow->radius = LIGHT_RADIUS;
	glow->intensity = 1.f;
	glow->castShadows = true;
}

void BlackoutBullet::Launch(const BlackoutCharacter& _shooter, const FVector2& _direction, const float& _damage, const bool& _real,
	const float& _speed, const float& _range)
{
	velocity = _direction * _speed;
	life = _range / std::max(1.f, _speed);
	damage = _damage;
	real = _real;
	shooterId = _shooter.GetId();
	team = _shooter.GetTeam();
	shooterPlayer = _shooter.GetPlayerId();
	origin = GetPosition();
	LookAt(GetPosition() + _direction);

	const Color _color = team == 0 ? Color(255, 230, 140) : Color(255, 110, 80);
	line->GetShapeObject()->GetShape()->SetFillColor(_color);
	glow->color = _color;
}

void BlackoutBullet::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);

	life -= static_cast<float>(_deltaTime);
	if (life <= 0.f)
	{
		Destroy();
		return;
	}

	const FVector2 _from = GetPosition();
	const FVector2 _to = _from + velocity * static_cast<float>(_deltaTime);

	// The nearest body on the way that is not the shooter.
	for (const RaycastHitInfo& _hit : GetLevel()->GetWorldPhysics().RaycastAll(_from, _to))
	{
		if (!_hit.hit || !_hit.actor || _hit.actor->GetId() == shooterId || _hit.actor->IsPendingDestroy()) continue;

		if (BlackoutCharacter* _character = Cast<BlackoutCharacter>(_hit.actor))
		{
			if (_character->IsDead()) continue;
			if (_character->GetTeam() != team && real)
			{
				// A network game: the machine of the shooter tells everybody (each one then hurts its copy the same way).
				BlackoutGameMode* _mode = BlackoutGameMode::Of(GetLevel());
				if (_mode && _mode->IsVersus()) _mode->SendHit(_character->GetPlayerId(), damage, origin, shooterPlayer);
				else _character->TakeDamage(damage, origin);	// it bleeds (BlackoutBleed)
			}
		}
		else
			BlackoutEffect(GetLevel(), ParticlePreset::Sparks, _hit.point);

		Destroy();
		return;
	}
	SetPosition(_to);
}

// ---- BlackoutDroppedWeapon

BlackoutDroppedWeapon::BlackoutDroppedWeapon(Level* _level)
	: Actor(_level)
{
	name = "BlackoutDroppedWeapon";

	picture = CreateComponent<SpriteComponent>(RectangleShapeData(BlackoutWeapon::Get(0).size, BlackoutWeapon::Get(0).texture));
	picture->SetZOrder(static_cast<std::uint8_t>(static_cast<int>(ZOrder::Actors) - 3));		// under the soldiers and the smokes

	// A faint light: a gun on the floor can be found in the dark.
	glow = CreateComponent<PointLightComponent>();
	glow->color = Color(200, 210, 230);
	glow->radius = 55.f;
	glow->intensity = 0.45f;
	glow->castShadows = false;
}

void BlackoutDroppedWeapon::Setup(const int& _dropId, const int& _weapon, const float& _angle)
{
	dropId = _dropId;
	weapon = std::clamp(_weapon, 0, BlackoutWeapon::COUNT - 1);
	const BlackoutWeapon& _stats = BlackoutWeapon::Get(weapon);
	picture->GetShapeObject()->SetTexture(_stats.texture);
	picture->GetShapeObject()->SetSize(_stats.size);
	picture->GetShapeObject()->GetShape()->SetFillColor(_stats.tint);
	SetRotation(Angle::FromDegrees(_angle));
}

}
