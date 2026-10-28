#include "SpatialSoundComponent.h"
#include "Core/AssetPack.h"
#include "Actors/Actor.h"

namespace
{
	// A sound that exists but plays nothing: a component without a file still has a valid sound to configure.
	Krampus::Sound MakeSilentSound()
	{
		auto _buffer = std::make_shared<sf::SoundBuffer>();
		const std::int16_t _samples[2] = { 0, 0 };
		(void)_buffer->loadFromSamples(_samples, 2, 1, 44100, { sf::SoundChannel::Mono });
		return Krampus::Sound(_buffer);
	}
}

Krampus::SpatialSoundComponent::SpatialSoundComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(SpatialSoundComponent);
	sound = MakeSilentSound();

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
	soundFile = _fileName;
	soundType = _type;

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
	ApplySpatialSettings();
}

void Krampus::SpatialSoundComponent::ApplySpatialSettings()
{
	SetMinDistance(400.0f);
	SetAttenuation(1.5f);
	SetSpatializationEnabled(true);
	SetRelativeToListener(false);
	SetDopplerFactor(0.1f);
	Set3DPosition(FVector3(GetPosition().x, 0.0f, GetPosition().y));
}

void Krampus::SpatialSoundComponent::SetSoundFile(const String& _fileName, const AudioExtensionType& _type)
{
	const Float _volume = GetVolume();
	const Bool _loop = IsLooping();
	const Float _pitch = GetPitch();
	const Float _attenuation = GetAttenuation();

	sound.Stop();

	const std::string _name = _fileName.StdString();
	const bool _exists = !_name.empty() && AssetPack::Exists(CONTENT_PATH + "Audio/" + _name + std::string(_type == AudioExtensionType::WAV ? ".wav" : ".mp3"));
	if (!_name.empty() && !_exists)
		LOG_WARNING("SpatialSoundComponent: sound file not found: " + _name);

	sound = _exists ? M_AUDIO.CreateSound(_fileName, _type) : MakeSilentSound();
	soundFile = _exists ? _fileName : String();
	soundType = _type;

	ApplySpatialSettings();
	SetVolume(_volume);
	SetLoop(_loop);
	SetPitch(_pitch);
	SetAttenuation(_attenuation);
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
