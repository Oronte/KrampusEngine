#include "SpatialSoundComponent.h"
#include "Actors/Actor.h"

Krampus::SpatialSoundComponent::SpatialSoundComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(SpatialSoundComponent);

	onOwnerMoveHandle = _owner->onMove.AddListener([this](FVector2 _newPos)
		{
			Set3DPosition(FVector3(_newPos.x, 0.0f, _newPos.y));
		});
	onOwnerRotateHandle = _owner->onRotate.AddListener([this](Angle _newRot)
		{
			FVector2 _dir;
			if (isTopDown)
				_dir = GetUpVector();
			else
				_dir = GetRightVector();
			SetDirection(FVector3(_dir.x, 0.0f, _dir.y));
		});
}

Krampus::SpatialSoundComponent::SpatialSoundComponent(Actor* _owner, const String& _fileName, const AudioExtensionType& _type)
	: Component(_owner)
{
	name = NAME_OF(SpatialSoundComponent);
	sound = M_AUDIO.CreateSound(_fileName, _type);

	onOwnerMoveHandle = _owner->onMove.AddListener([this](FVector2 _newPos)
		{
			Set3DPosition(FVector3(_newPos.x, 0.0f, _newPos.y));
		});
	onOwnerRotateHandle = _owner->onRotate.AddListener([this](Angle _newRot)
		{
			FVector2 _dir;
			if (isTopDown)
				_dir = GetUpVector();
			else
				_dir = GetRightVector();
			SetDirection(FVector3(_dir.x, 0.0f, _dir.y));
		});
}

void Krampus::SpatialSoundComponent::Construct()
{
	Component::Construct();

	previousPosition = GetPosition();

	SetMinDistance(400.0f);
	SetAttenuation(1.5f);
	SetSpatializationEnabled(true);
	SetRelativeToListener(false);
	SetDopplerFactor(0.1f);
}

void Krampus::SpatialSoundComponent::BeginPlay()
{
	Component::BeginPlay();

	if (playOnBeginPlay)
		Play();
}

void Krampus::SpatialSoundComponent::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	FVector2 _currentPosition = GetPosition();
	FVector2 _offset = _currentPosition - previousPosition;
	SetVelocity(FVector3(_offset.x, 0.0f, _offset.y));
	previousPosition = _currentPosition;
}
