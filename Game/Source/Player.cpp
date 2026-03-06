#include "Player.h"
#include "Managers/InputManager.h"
#include "Graphics/Mouse.h"

Player::Player(Level* _level)
	: Actor(_level)
{
	sprite = CreateComponent<SpriteComponent>(RectangleShapeData(FVector2(100.0f, 100.0f), "Player"));
	collision = CreateComponent<CollisionComponent>(CollisionChannel::Player, 
		CollisionChannel::Enemy | CollisionChannel::Wall | CollisionChannel::Trigger);
	rigidbody = CreateComponent<RigidbodyComponent>();
	inputs = CreateComponent<InputComponent>();
	animation = CreateComponent<AnimationComponent>();
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

	animation->AddAnimation("Idle", AnimationData(4, 0.5f, IRect(IVector2(0, 32 * 0), IVector2(32))));
	animation->AddAnimation("Move", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 1), IVector2(32))));
	animation->AddAnimation("Jump", AnimationData(3, 0.3f, IRect(IVector2(0, 32 * 2), IVector2(32)), false));
	animation->AddAnimation("Dash", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 3), IVector2(32))));
	animation->AddAnimation("Hit", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 4), IVector2(32)), false));
	animation->AddAnimation("Death", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 5), IVector2(32)), false));
	animation->AddAnimation("Sleep", AnimationData(4, 0.3f, IRect(IVector2(0, 32 * 6), IVector2(32))));

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
		});

	animation->StartAnimation();
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
		animation->SetCurrentAnimation("Move");
		animation->StartAnimation();
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
		animation->SetCurrentAnimation("Move");
		animation->StartAnimation();
	}
	transform.position += FVector2::Right() * moveSpeed * GetDeltaTime();
	transform.scale = FVector2(1.0f, 1.0f);
}

void Player::Stop()
{
	if (isJumping) return;
	isRunning = false;
	animation->SetCurrentAnimation("Idle");
	animation->StartAnimation();
}

void Player::Jump()
{
	if (isJumping) return;

	rigidbody->AddImpulse(FVector2::Up() * jumpForce);
	animation->SetCurrentAnimation("Jump");
	animation->StartAnimation();
	isJumping = true;
}
