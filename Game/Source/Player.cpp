#include "Player.h"
#include "Managers/InputManager.h"
#include "Graphics/Mouse.h"
#include "GameFramework/Level.h"

Player::Player(Level* _level)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(100.0f, 100.0f), "Player"));
	collision = CreateComponent<CollisionComponent>(CollisionChannel::Player, 
		CollisionChannel::Enemy | CollisionChannel::Wall | CollisionChannel::Trigger);
	rigidbody = CreateComponent<RigidbodyComponent>();
	inputs = CreateComponent<InputComponent>();
	animationSM = CreateComponent<AnimationComponentSM>();
	camera = CreateComponent<CameraComponent>();
	health = CreateComponent<HealthComponent>();
}

void Player::Construct()
{
	Actor::Construct();

	rigidbody->freezeMovementX = true;
	rigidbody->freezeRotation = true;
	rigidbody->restitution = 0.0f;
	rigidbody->BindCollisionResponse();

	inputs->Bind(&GetWorld()->GetInputManager()->D.onPerform, this, &Player::MoveRight);
	inputs->Bind(&GetWorld()->GetInputManager()->Q.onPerform, this, &Player::MoveLeft);
	inputs->Bind(&GetWorld()->GetInputManager()->D.onRelease, this, &Player::Stop);
	inputs->Bind(&GetWorld()->GetInputManager()->Q.onRelease, this, &Player::Stop);
	inputs->Bind(&GetWorld()->GetInputManager()->Space.onPress, this, &Player::Jump);

	AnimationSM* _idle = animationSM->AddAnimation("Idle", AnimationData(4, 0.5f, IRect(IVector2(0, 32 * 0), IVector2(32))));
	AnimationSM* _move = animationSM->AddAnimation("Move", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 1), IVector2(32))));
	AnimationSM* _jump = animationSM->AddAnimation("Jump", AnimationData(3, 0.3f, IRect(IVector2(0, 32 * 2), IVector2(32)), false));
	AnimationSM* _hit = animationSM->AddAnimation("Hit", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 4), IVector2(32)), false));
	AnimationSM* _death = animationSM->AddAnimation("Death", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 5), IVector2(32)), false));

	_idle->CreateLink(_move, [this]() -> bool
		{
			return isRunning;
		});
	_idle->CreateLink(_jump, [this]() -> bool
		{
			return isJumping;
		});
	_idle->CreateLink(_hit, [this]() -> bool
		{
			return isHit;
		});
	_idle->CreateLink(_death, [this]() -> bool
		{
			return isDying;
		});
	_move->CreateLink(_idle, [this]() -> bool
		{
			return !isRunning;
		});
	_move->CreateLink(_jump, [this]() -> bool
		{
			return isJumping;
		});
	_jump->CreateLink(_idle, [this]() -> bool
		{
			return !isJumping;
		});
	_jump->CreateLink(_hit, [this]() -> bool
		{
			return isHit;
		});
	_jump->CreateLink(_death, [this]() -> bool
		{
			return isDying;
		});
	_hit->CreateLink(_idle, [this]() -> bool
		{
			return !isHit;
		});

	camera->SetCurrent();
	camera->freezeRotation = true;

	collision->BindCollisionResolution();
}

void Player::BeginPlay()
{
	Actor::BeginPlay();

	handle = collision->onCollisionEnter.AddListener([this](CollisionInfo _info)
		{
			isJumping = false;
			Stop();
			if (_info.collision->ContainsCollisionChannel(_info.collision->channel, CollisionChannel::Enemy))
			{
				bool _isAlive = health->Damage(10.0f);
				if (_isAlive)
				{
					isHit = true;
					CreateTimer([this]() {isHit = false; }, 0.3f);
				}
				else
				{
					isDying = true;
					Stop();
				}
			}
		});

	deathHandle = health->onDeath.AddListener(this, &::Player::Die);

	animationSM->StartAnimation();
}

void Player::Tick(const float& _deltaTime)
{
	Super::Tick(_deltaTime);

	camera->SetCenter((transform.position + GetWorld()->GetMouse()->GetPosition() / 2.0f) / 2.0f);
}

void Player::MoveLeft()
{
	if (!canMove) return;
	if (!isRunning && !isJumping)
	{
		isRunning = true;
	}
	transform.position += FVector2::Left() * moveSpeed * GetDeltaTime();
	transform.scale = FVector2(-1.0f, 1.0f);
}

void Player::MoveRight()
{
	if (!canMove) return;
	if (!isRunning && !isJumping)
	{
		isRunning = true;
	}
	transform.position += FVector2::Right() * moveSpeed * GetDeltaTime();
	transform.scale = FVector2(1.0f, 1.0f);
}

void Player::Stop()
{
	if (isJumping) return;
	isRunning = false;
}

void Player::Jump()
{
	if (isJumping) return;

	rigidbody->AddImpulse(FVector2::Up() * jumpForce);
	isJumping = true;
}

void Player::Die()
{
	//RemoveComponent<CollisionComponent>();
	//RemoveComponent<RigidbodyComponent>();
	//RemoveComponent<InputComponent>();
	//RemoveComponent<HealthComponent>();

	//collision = nullptr;
	//rigidbody = nullptr;
	//inputs = nullptr;
	//health = nullptr;
	//deathHandle.~ListenerHandle();

	//CreateTimer([this]()
	//	{
	//		GetLevel()->Unload();
	//		GetLevel()->Load();
	//	}, 1.5f);

	Destroy();
}
