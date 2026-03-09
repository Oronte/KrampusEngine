#include "Enemy.h"
#include "GameFramework/Level.h"
#include "DeathFX.h"

Enemy::Enemy(Level* _level, Actor* _player)
	: Actor(_level), player(_player)
{
	sprite = CreateComponent<SpriteComponent>(CircleShapeData(50.0f, "FlyingEnemy"));
	animation = CreateComponent<AnimationComponent>();
	collision = CreateComponent<CollisionComponent>(CollisionChannel::Enemy,
		 CollisionChannel::Projectile | CollisionChannel::Wall | CollisionChannel::Trigger | CollisionChannel::Player);
	rigidbody = CreateComponent<RigidbodyComponent>();
	health = CreateComponent<HealthComponent>();
}

void Enemy::Construct()
{
	Super::Construct();

	rigidbody->isKinematic = true;
	rigidbody->SetActive(false);
	rigidbody->BindCollisionResponse();
	animation->AddAnimation("Flying", AnimationData(4, 0.5f, IRect(IVector2(), IVector2(81, 71))));
	animation->StartAnimation();

	handle = collision->onCollisionEnter.AddListener([this](CollisionInfo _info)
		{
			HealthComponent* _compo = _info.collision->GetOwner()->GetComponent<HealthComponent>();
			if ((_info.collision->channel == CollisionChannel::Player) && _compo)
			{
				Die();
			}
		});

	deathHandle = health->onDeath.AddListener(this, &Enemy::Die);
}

void Enemy::Tick(const float& _deltaTime)
{
	Super::Tick(_deltaTime);

	if (player)
		transform.position += transform.position.DirectionTo(player->transform.position).Normalized() * 100.0f * _deltaTime;
}

void Enemy::Die()
{
	rigidbody->isKinematic = false;
	rigidbody->SetActive(true);
	rigidbody->AddImpulse(FVector2(FMath::RandomRange(-50.0f, 50.0f), -FMath::RandomRange(500.0f, 800.0f)),
		FVector2(200.0f));
	collision->mask = CollisionChannel::Wall | CollisionChannel::Enemy | CollisionChannel::Trigger;
	CreateTimer([this]()
		{
			GetLevel()->SpawnActorAt<DeathFX>(Transform(transform.position, Angle()));
			Destroy();
		}, FMath::RandomRange(0.5f, 3.0f));
}
