#include "Character.h"
#include "Managers/InputManager.h"

Krampus::Character::Character(Level* _level, const RectangleShapeData& _data)
	: PhysicsActor(_level, _data)
{
	//animation = CreateComponent<AnimationComponent>();
	audioListener = CreateComponent<AudioListenerComponent>();
	camera = CreateComponent<CameraComponent>();
	camera->attachedToOwner = true;
	rigidbody->freezeRotation = true;
}

void Krampus::Character::BeginPlay()
{
	PhysicsActor::BeginPlay();

	handles.push_back(GetWorld()->GetInputManager()->D.onPerform.AddListener(this, &Character::MoveRight));
	handles.push_back(GetWorld()->GetInputManager()->Q.onPerform.AddListener(this, &Character::MoveLeft));
}

void Krampus::Character::MoveRight()
{
	rigidbody->AddForce(FVector2(100.0f, 0.0f));
}

void Krampus::Character::MoveLeft()
{
	rigidbody->AddForce(FVector2(-100.0f, 0.0f));
}
