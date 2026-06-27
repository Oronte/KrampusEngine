#include "Enemy.h"
#include "GameFramework/Level.h"
#include "DeathFX.h"
#include "Managers/AudioManager.h"

Enemy::Enemy(Level* _level, Player* _player)
	: Actor(_level), player(_player)
{
	sprite = CreateComponent<SpriteComponent>(CircleShapeData(50.0f, "FlyingEnemy"));
	animation = CreateComponent<AnimationComponent>();
	collision = CreateComponent<CollisionComponent>(CollisionChannel::Enemy,
		 CollisionChannel::Projectile | CollisionChannel::Wall | CollisionChannel::Trigger | CollisionChannel::Player);
	rigidbody = CreateComponent<RigidbodyComponent>();
	health = CreateComponent<HealthComponent>();

	enemySound = M_AUDIO.CreateSound("WingsSound", AudioExtensionType::MP3);
	hurtSound = M_AUDIO.CreateSound("EnemyHurt", AudioExtensionType::MP3);
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
			if (CollisionComponent::ContainsCollisionChannel(_info.collision->channel, CollisionChannel::Player).And(_compo))
			{
				Die();
			}
		});

	hurtSoundHandle = health->onHealthUpdate.AddListener([this](Int _health)
		{
			hurtSound.Play();
		});
	deathHandle = health->onDeath.AddListener(this, &Enemy::Die);

	soundHandle = animation->GetCurrentAnimation()->notifies[1].AddListener([this]()
		{
			enemySound.Play();
		});

	enemySound.SetSpatializationEnabled(true);
	enemySound.SetRelativeToListener(false);
	enemySound.SetMinDistance(50.f);
	enemySound.SetMaxDistance(500.f);
	enemySound.SetAttenuation(1.f);
	enemySound.SetDopplerFactor(1.f);
	enemySound.SetMinGain(0.f);
	enemySound.SetMaxGain(1.f);
}

void Enemy::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);

	enemySound.SetPosition(FVector3(GetActorPosition().x, 0.0f, GetActorPosition().y));

	if (player)
		Move(GetActorPosition().DirectionTo(player->GetActorPosition()).Normalized() * 100.0f * _deltaTime);
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
			GetLevel()->SpawnActorAt<DeathFX>(Transform(GetActorPosition(), Angle()));
			Destroy();
		}, FMath::RandomRange(0.5f, 3.0f));

	player->AddScore(IMath::RandomRange(100, 500));
}
