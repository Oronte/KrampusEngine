#include "BlackoutHero.h"
#include "BlackoutGameMode.h"
#include "BlackoutGrenade.h"
#include "BlackoutKeys.h"
#include "Core/Engine.h"
#include "Network/NetworkManager.h"

namespace Krampus
{

BlackoutHero::BlackoutHero(Level* _level)
	: BlackoutCharacter(_level)
{
	name = "Hero";
	team = 0;

	// The camera follows him, without turning with him.
	camera = CreateComponent<CameraComponent>(FVector2(1440.f, 810.f));
	camera->attachedToOwner = true;
	camera->freezeRotation = true;

	flashlight = CreateComponent<SpotLightComponent>();
	flashlight->radius = 750.f;
	flashlight->angle = 70.f;
	flashlight->softness = 0.35f;
	flashlight->intensity = 1.2f;

	aura = CreateComponent<PointLightComponent>();
	aura->radius = 190.f;
	aura->intensity = 0.8f;
	aura->color = Color(170, 180, 210);
}

void BlackoutHero::BeginPlay()
{
	BlackoutCharacter::BeginPlay();
	NetworkManager* _net = GetWorld()->GetNetwork();
	const std::string _id = std::to_string(_net->GetLocalId());
	SetLoadout(_net->GetShared<int>("primary_" + _id, -1), _net->GetShared<int>("sidearm_" + _id, BlackoutWeapon::PISTOL));		// what was bought
	SetShield(static_cast<float>(_net->GetShared<int>("shield_" + _id, 0)));
	grenadesLeft = 0;			// bought in the shop
	flashesLeft = 0;
	smokesLeft = 0;
	spawnPosition = GetPosition();
	camera->SetCenter(GetPosition());
	// A network game: SetupPlayer chooses the camera (the local player's), not every hero that begins.
	if (!GetWorld()->GetNetwork()->IsOnline()) camera->SetCurrent();
}

Color BlackoutHero::TeamColor(const int& _team)
{
	return _team == 0 ? Color(130, 175, 255) : Color(255, 150, 110);
}

void BlackoutHero::SetupPlayer(const int& _playerId, const int& _team, const bool& _local, const bool& _friend)
{
	playerId = _playerId;
	team = _team;
	localPlayer = _local;
	friendly = _friend;
	spawnPosition = GetPosition();		// placed by the game mode before
	NetworkManager* _net = GetWorld()->GetNetwork();
	const std::string _id = std::to_string(_playerId);
	SetLoadout(_net->GetShared<int>("primary_" + _id, -1), _net->GetShared<int>("sidearm_" + _id, BlackoutWeapon::PISTOL));
	SetShield(static_cast<float>(_net->GetShared<int>("shield_" + _id, 0)));		// kept from the last round

	tint = TeamColor(_team);
	body->GetShapeObject()->GetShape()->SetFillColor(tint);

	if (_local)
	{
		camera->SetCenter(GetPosition());
		camera->SetCurrent();
	}
	else
		physics->SetKinematic();		// moved by its NetworkSyncComponent, not by forces

	// The other team is only seen in our lights: its own lights are off here.
	if (!_friend)
	{
		flashlight->intensity = 0.f;
		aura->intensity = 0.f;
	}
}

FVector2 BlackoutHero::MouseInWorld() const
{
	const Window& _window = GetWorld()->GetWindowRef();
	return _window.MapPixelToCoords(_window.GetMousePosition(), *camera->GetView());
}

void BlackoutHero::ApplyStats(const int& _weapon)
{
	const BlackoutWeapon& _stats = BlackoutWeapon::Get(_weapon);
	weapon = std::clamp(_weapon, 0, BlackoutWeapon::COUNT - 1);
	bulletDamage = _stats.damage;
	fireDelay = _stats.delay;
	pellets = _stats.pellets;
	bulletSpeed = _stats.speed;
	bulletRange = _stats.range;
	magazineSize = _stats.magazine;
	reloadSeconds = _stats.reload;
	spread = _stats.spread;
	automatic = _stats.automatic;
	SetGunLook(weapon);
	reloadLeft = 0.f;

	// The snipers see farther: a wider view and a longer, narrower beam.
	const bool _sniper = _stats.category == BlackoutWeapon::Sniper;
	camera->SetSize(FVector2(_stats.view, _stats.view * 9.f / 16.f));
	flashlight->radius = _sniper ? 1050.f : 750.f;
	flashlight->angle = _sniper ? 50.f : 70.f;
}

void BlackoutHero::SetLoadout(const int& _primary, const int& _sidearm)
{
	primary = _primary >= 0 && _primary < BlackoutWeapon::COUNT && !BlackoutWeapon::Get(_primary).IsSidearm() ? _primary : -1;
	sidearm = _sidearm >= 0 && _sidearm < BlackoutWeapon::COUNT && BlackoutWeapon::Get(_sidearm).IsSidearm() ? _sidearm : BlackoutWeapon::PISTOL;
	slotAmmo[0] = primary >= 0 ? BlackoutWeapon::Get(primary).magazine : 0;
	slotAmmo[1] = BlackoutWeapon::Get(sidearm).magazine;
	slot = -1;
	Equip(primary >= 0 ? 0 : 1);
}

void BlackoutHero::GiveWeapon(const int& _weapon)
{
	const int _clamped = std::clamp(_weapon, 0, BlackoutWeapon::COUNT - 1);
	const int _slot = BlackoutWeapon::Get(_clamped).IsSidearm() ? 1 : 0;
	if (slot >= 0) slotAmmo[slot] = ammo;		// the other gun keeps its magazine
	(_slot == 0 ? primary : sidearm) = _clamped;
	slotAmmo[_slot] = BlackoutWeapon::Get(_clamped).magazine;
	slot = -1;
	Equip(_slot);
}

void BlackoutHero::Equip(const int& _slot)
{
	if (_slot == 0 && primary < 0) return;
	if (_slot == slot) return;
	if (slot >= 0) slotAmmo[slot] = ammo;		// the gun put away keeps its magazine
	slot = std::clamp(_slot, 0, 1);
	ApplyStats(slot == 0 ? primary : sidearm);
	ammo = slotAmmo[slot];
	fireCooldown = std::max(fireCooldown, 0.25f);	// taking a gun in hand takes a moment
}

void BlackoutHero::SwitchSlot(const int& _slot)
{
	if (_slot == slot || (_slot == 0 && primary < 0)) return;
	Equip(_slot);
	NetworkManager* _net = GetWorld()->GetNetwork();
	if (_net->IsOnline()) _net->Send("BO_Slot", NetworkManager::Args(playerId, slot), NetTarget::Others);
}

void BlackoutHero::AddGrenade()
{
	grenadesLeft = std::min(grenadesLeft + 1, BlackoutWeapon::MAX_GRENADES);
}

void BlackoutHero::AddFlash()
{
	flashesLeft = std::min(flashesLeft + 1, BlackoutWeapon::MAX_FLASHES);
}

FVector2 BlackoutHero::KeepInBuyZone(const FVector2& _direction, const float& _radius)
{
	const FVector2 _offset = GetPosition() - spawnPosition;
	const float _distance = _offset.Length();
	if (_distance < 1.f) return _direction;
	const FVector2 _out = _offset / _distance;

	// Out of the zone (pushed, or the zone was just made): back on its edge.
	if (_distance > _radius) SetPosition(spawnPosition + _out * _radius);

	// On the edge: only along it, or back in.
	const float _outward = _direction.x * _out.x + _direction.y * _out.y;
	if (_distance >= _radius - 4.f && _outward > 0.f) return _direction - _out * _outward;
	return _direction;
}

void BlackoutHero::Reload()
{
	if (IsReloading() || ammo >= magazineSize) return;
	reloadLeft = reloadSeconds;
}

void BlackoutHero::Tick(const Float& _deltaTime)
{
	BlackoutCharacter::Tick(_deltaTime);
	if (IsDead()) return;

	const float _delta = static_cast<float>(_deltaTime);
	if (!localPlayer) return;		// moved by its NetworkSyncComponent; its steps come with BO_Step

	throwCooldown = std::max(0.f, throwCooldown - _delta);
	if (reloadLeft > 0.f)
	{
		reloadLeft -= _delta;
		if (reloadLeft <= 0.f)
		{
			reloadLeft = 0.f;
			ammo = magazineSize;
		}
	}

	// The keyboard of the whole computer is polled: only the window that has the focus plays. Typing in the chat, planting or
	// defusing the charge: the hero does not move nor shoot.
	BlackoutGameMode* _mode = BlackoutGameMode::Of(GetLevel());
	if (!GetWorld()->GetWindowRef().HasFocus() || (_mode && _mode->IsMoveBlocked()))
	{
		Walk(FVector2(), 0.f);
		fireHeld = true;
		throwHeld = true;
		flashHeld = true;
		smokeHeld = true;
		walking = false;
		return;
	}

	using Key = sf::Keyboard::Key;
	const auto _down = [](const std::initializer_list<Key> _keys)
	{
		for (const Key _key : _keys)
			if (sf::Keyboard::isKeyPressed(_key)) return true;
		return false;
	};

	FVector2 _direction;
	// The keys chosen in the menu (BlackoutKeys), and the arrows.
	if (BlackoutKeys::Down(BlackoutKeys::MoveUp) || _down({ Key::Up }))			_direction.y -= 1.f;
	if (BlackoutKeys::Down(BlackoutKeys::MoveDown) || _down({ Key::Down }))		_direction.y += 1.f;
	if (BlackoutKeys::Down(BlackoutKeys::MoveLeft) || _down({ Key::Left }))		_direction.x -= 1.f;
	if (BlackoutKeys::Down(BlackoutKeys::MoveRight) || _down({ Key::Right }))	_direction.x += 1.f;

	// Shift: walk, slower and silent. Running makes steps the other side hears (an echo, the enemies of the solo game come).
	walking = BlackoutKeys::Down(BlackoutKeys::Walk);
	if (_direction.Length() > 0.f) _direction = _direction.Normalized();
	if (_mode && _mode->IsBuyPhase()) _direction = KeepInBuyZone(_direction, static_cast<float>(_mode->buyZoneRadius));
	const bool _moving = _direction.Length() > 0.05f;
	Walk(_moving ? _direction : FVector2(), static_cast<float>(moveSpeed) * (walking ? WALK_FACTOR : 1.f));
	stepTime -= _delta;
	if (_moving && !walking && stepTime <= 0.f)
	{
		stepTime = STEP_INTERVAL;
		Step();
	}

	// Moving spoils the aim (smoothed: the red cone grows and shrinks).
	const float _wantSpread = !_moving ? 1.f : (walking ? WALK_SPREAD : RUN_SPREAD);
	const float _wantAdd = !_moving ? 0.f : (walking ? WALK_SPREAD_ADD : RUN_SPREAD_ADD);
	const float _blend = std::min(1.f, _delta * 7.f);
	moveSpread += (_wantSpread - moveSpread) * _blend;
	moveSpreadAdd += (_wantAdd - moveSpreadAdd) * _blend;

	// The gun (on the right of the head) points at the mouse.
	const FVector2 _mouse = MouseInWorld();
	if ((_mouse - GetPosition()).Length() > 4.f) AimAt(_mouse);

	// 1: the primary gun, 2: the sidearm (by default the keys of the top row, whatever the keyboard layout).
	const bool _one = BlackoutKeys::Down(BlackoutKeys::Primary), _two = BlackoutKeys::Down(BlackoutKeys::Sidearm);
	if ((_one || _two) && !slotKeyHeld) SwitchSlot(_one ? 0 : 1);
	slotKeyHeld = _one || _two;

	// Shopping (buy phase, shop open): no shot, no throw.
	if (_mode && _mode->IsFireBlocked())
	{
		fireHeld = true;
		throwHeld = true;
		flashHeld = true;
		smokeHeld = true;
		return;
	}

	if (BlackoutKeys::Down(BlackoutKeys::Reload)) Reload();
	// The rifle shoots while the button is down; the shotgun and the sniper once per click.
	const bool _trigger = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
	if (!_trigger) triggerReady = true;		// a click held from the menu does not shoot
	if (_trigger && triggerReady && (automatic || !fireHeld) && !IsReloading())
	{
		if (ammo <= 0) Reload();
		else if (Fire(_mouse, GetSpread()))
		{
			--ammo;
			// The others see the same bullets (their copies hurt nobody: this machine tells the hits).
			NetworkManager* _net = GetWorld()->GetNetwork();
			if (_net->IsOnline())
			{
				std::vector<PropertyValue> _args = NetworkManager::Args(playerId, lastMuzzle, bulletSpeed, bulletRange);
				for (const FVector2& _direction : lastDirections) _args.push_back(_direction);
				_net->Send("BO_Shot", _args, NetTarget::Others);
			}
		}
	}
	fireHeld = _trigger;

	// One grenade per press of the right button or its key (G).
	const bool _throw = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) || BlackoutKeys::Down(BlackoutKeys::Grenade);
	if (_throw && !throwHeld) ThrowGrenade(_mouse, BlackoutGrenade::Frag);
	throwHeld = _throw;

	// One flash per press of its key (F).
	const bool _flash = BlackoutKeys::Down(BlackoutKeys::Flash);
	if (_flash && !flashHeld) ThrowGrenade(_mouse, BlackoutGrenade::Flash);
	flashHeld = _flash;

	// One smoke per press of its key (C).
	const bool _smoke = BlackoutKeys::Down(BlackoutKeys::Smoke);
	if (_smoke && !smokeHeld) ThrowGrenade(_mouse, BlackoutGrenade::Smoke);
	smokeHeld = _smoke;
}

void BlackoutHero::Step()
{
	// A faint ring here: you see the noise you make. The others: BO_Step (an echo for the other team), or the solo enemies hear it.
	BlackoutGameMode* _mode = BlackoutGameMode::Of(GetLevel());
	if (_mode) _mode->HeroStep(*this);
	NetworkManager* _net = GetWorld()->GetNetwork();
	if (_net->IsOnline()) _net->Send("BO_Step", NetworkManager::Args(playerId, GetPosition()), NetTarget::Others, NetChannel::Unreliable);
}

void BlackoutHero::ThrowGrenade(const FVector2& _target, const int& _kind)
{
	int& _left = _kind == BlackoutGrenade::Flash ? flashesLeft : (_kind == BlackoutGrenade::Smoke ? smokesLeft : grenadesLeft);
	if (_left <= 0 || throwCooldown > 0.f) return;
	--_left;
	throwCooldown = 0.6f;

	// From the hand; from the head when the hand is in a wall (the wall then stops it).
	FVector2 _start = GetPosition() + GetRightVector() * 30.f;
	if (GetLevel()->GetWorldPhysics().Raycast(GetPosition(), _start, WorldPhysics::AllChannels(), this).hit) _start = GetPosition();

	BlackoutGrenade* _grenade = GetLevel()->SpawnActor<BlackoutGrenade>();
	_grenade->SetPosition(_start);
	_grenade->Throw(*this, _target, true, _kind);

	NetworkManager* _net = GetWorld()->GetNetwork();
	if (_net->IsOnline()) _net->Send("BO_Grenade", NetworkManager::Args(playerId, _start, _target, _kind), NetTarget::Others);
	LOG(VerbosityType::Log, std::string("Blackout: ") + (_kind == BlackoutGrenade::Flash ? "flash" : (_kind == BlackoutGrenade::Smoke ? "smoke" : "grenade")) +
		" thrown (" + std::to_string(_left) + " left)");
}

void BlackoutHero::TakeDamage(const float& _amount, const FVector2& _from)
{
	BlackoutCharacter::TakeDamage(_amount, _from);
	if (!localPlayer) return;
	if (BlackoutGameMode* _mode = BlackoutGameMode::Of(GetLevel()))
		_mode->HeroHurt();
}

void BlackoutHero::Abandon()
{
	if (IsDead()) return;
	health = 0.f;
	physics->SetLinearVelocity(FVector2());
	Die();
}

void BlackoutHero::Die()
{
	// The lights go out, the body stays on the floor.
	flashlight->intensity = 0.f;
	aura->radius = 90.f;
	aura->intensity = friendly ? 0.4f : 0.f;
	tint = Color(110, 60, 60);
	body->GetShapeObject()->GetShape()->SetFillColor(tint);
	physics->Disable();
	LOG(VerbosityType::Log, "Blackout: hero " + std::to_string(playerId) + " killed");
}

}
