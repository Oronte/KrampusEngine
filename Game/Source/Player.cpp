#include "Player.h"
#include "Managers/InputManager.h"
#include "Graphics/Mouse.h"
#include "GameFramework/Level.h"
#include "Managers/AudioManager.h"
#include "Managers/LevelManager.h"
#include "MainMenu.h"

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
	listener = CreateComponent<AudioListenerComponent>();

	footStep = M_AUDIO.CreateSound("Footstep", AudioExtensionType::MP3);
	hurtSound = M_AUDIO.CreateSound("PlayerHurt", AudioExtensionType::MP3);

	//auto _save = SaveManager::LoadOrThrow(String("PlayerSave"));
	//std::optional<Player> _player = _save.GetAs<Player>("Player");

	auto _saveOpt = SaveManager::TryLoad("PlayerSave");
	SaveNode _testnode;
	SaveResult _r = SaveManager::Load("PlayerSave", _testnode);
	if (_r == SaveResult::Success)
	{
		score = _testnode.Get<Int>("PlayerScore", Int(10));
		LOG_MSG(score);
	}
	else LOG_WARNING((int)_r);
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

	inputs->Bind(&GetWorld()->GetInputManager()->M.onPress, [this]() {LOG_MSG("Test"); });
	inputs->Bind(&GetWorld()->GetInputManager()->W.onPress, [this]() {LOG_WARNING("Test"); });
	inputs->Bind(&GetWorld()->GetInputManager()->E.onPress, [this]() {LOG_ERROR("Test"); });

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

	footstepHandle = _move->notifies[2].AddListener([this]()
		{
			footStep.Play();
		});

	hurtHandle = health->onHealthUpdate.AddListener([this](Int _health)
		{
			hurtSound.Play();
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
			else
			{
				isJumping = false;
				Stop();
				footStep.Play();
			}
		});

	deathHandle = health->onDeath.AddListener(this, &Player::Die);

	animationSM->StartAnimation();
}

void Player::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);

	//LOG_MSG("This is a message log.");
	//LOG_WARNING("This is a warning log.");
	//LOG_ERROR("This is a error log.");
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
	RemoveComponent<RigidbodyComponent>();
	RemoveComponent<InputComponent>();
	collision->channel = CollisionChannel::None;

	CreateTimer([this]()
		{
			GetWorld()->GetLevelManager()->SetLevel<MainMenu>();
		}, 3.0f);

	//SaveNode _playerSave;
	//_playerSave.Set("Player", *this);
	//SaveManager::Save("PlayerSave", _playerSave);
	 
	SaveNode _playerSave;
	_playerSave.Set("PlayerScore", score);
	SaveResult _result = SaveManager::Save("PlayerSave", _playerSave);
	if (_result != SaveResult::Success)
		LOG_WARNING(CAST(int, _result));

	onDeath.Broadcast();
}

void Player::Save(SaveNode& _node) const
{
	_node.Set("Score", score);
}

void Player::Load(const SaveNode& _node)
{
	score = _node.Get<Int>("Score");
}
