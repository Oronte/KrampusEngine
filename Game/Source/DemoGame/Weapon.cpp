#include "Weapon.h"
#include "Managers/InputManager.h"
#include "Bullet.h"
#include "GameFramework/Level.h"
#include "Graphics/Mouse.h"
#include "Managers/AudioManager.h"

Weapon::Weapon(Level* _level, Player* _player)
	: Actor(_level)
{
	player = _player;
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(36.0f, 21.0f) * 2.5f, "Gun"));
	inputs = CreateComponent<InputComponent>();

	shootSound = M_AUDIO.CreateSound("GunSound", AudioExtensionType::MP3);
}

void Weapon::Construct()
{
	Super::Construct();

	inputs->Bind(&GetWorld()->GetInputManager()->MouseLeftClick.onPerform, this, &Weapon::Shoot);
	handle = player->onDeath.AddListener([this]()
		{
			RemoveComponent<InputComponent>();
		});
}

void Weapon::Shoot()
{
	if (!canShoot) return;

	canShoot = false;

	CreateTimer([this]()
		{
			canShoot = true;
		}, 1.0f / bulletsPerSec);

	FVector2 _dir = GetActorPosition().DirectionTo(GetWorld()->GetMouse()->GetPosition());
	_dir += FVector2::RandomRange(-1.0f, 1.0f).Normalized() * spread;

	Bullet* _bullet = GetLevel()->SpawnActorAt<Bullet>(
		Transform(GetActorPosition() + GetActorRightVector() * 36.0f * 2.0f + GetActorUpVector() * 10.0f, GetAngleTo(GetWorld()->GetMouse()->GetPosition())),
		_dir.Normalized());

	shootSound.Play();
}
