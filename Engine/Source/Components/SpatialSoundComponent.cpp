#include "SpatialSoundComponent.h"
#include "Core/AssetPack.h"
#include "Actors/Actor.h"

namespace
{
	constexpr float SPATIAL_MIN_DISTANCE	= 400.f;	// full volume inside this radius (pixels)
	constexpr float SPATIAL_ATTENUATION		= 1.5f;
	constexpr float SPATIAL_DOPPLER_FACTOR	= 0.1f;		// velocities are pixels per frame: a full Doppler would be far too strong

	// A sound that exists but plays nothing: a component without a file still has a valid sound to configure.
	Krampus::Sound MakeSilentSound()
	{
		auto _buffer = std::make_shared<sf::SoundBuffer>();
		const std::int16_t _samples[2] = { 0, 0 };
		(void)_buffer->loadFromSamples(_samples, 2, 1, 44100, { sf::SoundChannel::Mono });
		return Krampus::Sound(_buffer);
	}

	// SFML listens in 3D (X right, Y up, Z forward): the 2D plane of the game is X/Z.
	Krampus::FVector3 ToAudioSpace(const Krampus::FVector2& _vector)
	{
		return Krampus::FVector3(_vector.x, 0.f, _vector.y);
	}
}

Krampus::SpatialSoundComponent::SpatialSoundComponent(Actor* _owner)
	: Component(_owner), sound(MakeSilentSound())
{
	name = NAME_OF(SpatialSoundComponent);
	FollowOwner();
}

Krampus::SpatialSoundComponent::SpatialSoundComponent(Actor* _owner, const String& _fileName, const AudioExtensionType& _type)
	: Component(_owner), sound(M_AUDIO.CreateSound(_fileName, _type)), soundFile(_fileName), soundType(_type)
{
	name = NAME_OF(SpatialSoundComponent);
	FollowOwner();
}

void Krampus::SpatialSoundComponent::FollowOwner()
{
	onOwnerMoveHandle = owner->onMove.AddListener([this](FVector2 _newPosition) { sound.SetPosition(ToAudioSpace(_newPosition)); });
	onOwnerRotateHandle = owner->onRotate.AddListener([this](Angle) { UpdateDirection(); });
}

void Krampus::SpatialSoundComponent::UpdateDirection()
{
	sound.SetDirection(ToAudioSpace(isTopDown ? GetUpVector() : GetRightVector()));
}

void Krampus::SpatialSoundComponent::Construct()
{
	Component::Construct();
	previousPosition = GetPosition();
	UpdateDirection();
	ApplySpatialSettings();
}

void Krampus::SpatialSoundComponent::ApplySpatialSettings()
{
	sound.SetMinDistance(SPATIAL_MIN_DISTANCE);
	sound.SetAttenuation(SPATIAL_ATTENUATION);
	sound.SetSpatializationEnabled(true);
	sound.SetRelativeToListener(false);
	sound.SetDopplerFactor(SPATIAL_DOPPLER_FACTOR);
	sound.SetPosition(ToAudioSpace(GetPosition()));
}

void Krampus::SpatialSoundComponent::SetSoundFile(const String& _fileName, const AudioExtensionType& _type)
{
	const float _volume = sound.GetVolume();
	const bool _loop = sound.IsLooping();
	const float _pitch = sound.GetPitch();
	const float _attenuation = sound.GetAttenuation();
	sound.Stop();

	const bool _exists = !_fileName.empty() && AssetPack::Exists(CONTENT_PATH + "Audio/" + _fileName + (_type == AudioExtensionType::WAV ? ".wav" : ".mp3"));
	if (!_fileName.empty() && !_exists)
		LOG_WARNING("SpatialSoundComponent: sound file not found: " + _fileName);

	sound = _exists ? M_AUDIO.CreateSound(_fileName, _type) : MakeSilentSound();
	soundFile = _exists ? _fileName : String();
	soundType = _type;

	ApplySpatialSettings();
	sound.SetVolume(_volume);
	sound.SetLoop(_loop);
	sound.SetPitch(_pitch);
	sound.SetAttenuation(_attenuation);
}

void Krampus::SpatialSoundComponent::Play()
{
	LOG(VerbosityType::Verbose, "Sound: play '" + soundFile + "' on " + GetName());
	sound.Play();
}

void Krampus::SpatialSoundComponent::Stop()
{
	LOG(VerbosityType::Verbose, "Sound: stop '" + soundFile + "' on " + GetName());
	sound.Stop();
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
	const FVector2 _position = GetPosition();
	sound.SetVelocity(ToAudioSpace(_position - previousPosition));
	previousPosition = _position;
}
