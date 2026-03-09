#include "Bullet.h"
#include "Managers/TextureManager.h"
#include "Graphics/Mouse.h"
#include "HealthComponent.h"

Bullet::Bullet(Level* _level, const FVector2& _direction)
	: Actor(_level), direction(_direction)
{
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(32) * 3.0f, "Bullet"));
	animation = CreateComponent<AnimationComponent>();
	collision = CreateComponent<CollisionComponent>(FVector2(23.0f, 6.0f) * 3.0f, CollisionChannel::Projectile,
		CollisionChannel::Enemy | CollisionChannel::Player | CollisionChannel::Wall | CollisionChannel::Trigger);
}

void Bullet::Construct()
{
	Super::Construct();

	M_TEXTURE.SetTextureRect(sprite->GetShapeObject()->GetShape(), IRect(IVector2::Zero(), IVector2(32)));
	animation->AddAnimation("Explode", AnimationData(5, 0.25f, IRect(IVector2::Zero(), IVector2(32)), false));
	handle = collision->onCollisionEnter.AddListener(this, &Bullet::OnCollision);
}

void Bullet::BeginPlay()
{
	Super::BeginPlay();

	lifeSpanTimer = CreateTimer([this]()
		{
			OnCollision(CollisionInfo());
		}, lifeSpan);
}

void Bullet::Tick(const float& _deltaTime)
{
	Super::Tick(_deltaTime);

	Move(_deltaTime);
}

void Bullet::Move(float _deltaTime)
{
	if (!canMove) return;

	transform.position += direction.Normalized() * moveSpeed * _deltaTime;
}

void Bullet::OnCollision(CollisionInfo _info)
{
	if (isDying) return;
	isDying = true;
	if (lifeSpanTimer)
	{
		lifeSpanTimer->Stop();
		lifeSpanTimer = nullptr;
	}
	canMove = false;
	animation->StartAnimation();
	CreateTimer([this]()
		{
			animation->StopAnimation();
			Destroy();
		}, 0.25f);

	CollisionComponent* _collision = _info.collision;
	if (!_collision) return;
	Actor* _actor = _collision->GetOwner();
	if (!_actor) return;
	if (HealthComponent* _compo = _actor->GetComponent<HealthComponent>())
	{
		_compo->Damage(damage);
	}
}
