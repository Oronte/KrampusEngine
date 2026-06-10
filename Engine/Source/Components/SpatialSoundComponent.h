#pragma once
#include "Component.h"
#include "Managers/AudioManager.h"

namespace Krampus
{

	class SpatialSoundComponent : public Component
	{
		Sound sound;

		FVector2 previousPosition;
		Event<FVector2>::ListenerHandle onOwnerMoveHandle;
		Event<Angle>::ListenerHandle onOwnerRotateHandle;

	public:
		Bool isTopDown			= false;
		Bool playOnBeginPlay	= false;

	public:
		SpatialSoundComponent(Actor* _owner);
		SpatialSoundComponent(Actor* _owner, const String& _fileName, const AudioExtensionType& _type);

		virtual void Construct() override;
		virtual void BeginPlay() override;
		virtual void Tick(const Float& _deltaTime) override;

		inline virtual Float GetVolume() const 
		{
			return sound.GetVolume();
		}
		inline virtual void SetVolume(const Float& _volume) 
		{
			sound.SetVolume(_volume);
		}

		inline virtual void Play() 
		{
			sound.Play();
		}
		inline virtual void Pause() 
		{
			sound.Pause();
		}
		inline virtual void Stop() 
		{
			sound.Stop();
		}

		inline virtual Bool IsLooping() const 
		{
			return sound.IsLooping();
		}
		inline virtual void SetLoop(const Bool& _loop) 
		{
			sound.SetLoop(_loop);
		}

		inline virtual Float GetPitch() const 
		{
			return sound.GetPitch();
		}
		inline virtual void SetPitch(const Float& _pitch) 
		{
			sound.SetPitch(_pitch);
		}
		inline virtual void AddPitch(const Float& _pitch) 
		{
			SetPitch(GetPitch() + _pitch);
		}

		inline virtual Bool IsAvaliable() const 
		{
			return !IsPlaying();
		}
		inline virtual Bool IsPlaying() const 
		{
			return sound.IsPlaying();
		}
		inline virtual Bool IsPaused() const 
		{
			return sound.IsPaused();
		}
		inline virtual Bool IsStopped() const 
		{
			return sound.IsStopped();
		}

		inline virtual void SetAttenuation(Float _attenuation)
		{
			sound.SetAttenuation(_attenuation);
		}
		inline virtual Float GetAttenuation() const
		{
			return sound.GetAttenuation();
		}

		inline void SetSpatializationEnabled(Bool _enable) const
		{
			sound.SetSpatializationEnabled(_enable);
		}
		inline Bool IsSpatializationEnabled() const
		{
			return sound.IsSpatializationEnabled();
		}

		inline void SetRelativeToListener(Bool _relative) const
		{
			sound.SetRelativeToListener(_relative);
		}
		inline Bool IsRelativeToListener() const
		{
			return sound.IsRelativeToListener();
		}

		inline void SetVelocity(const FVector3& _velocity) const
		{
			sound.SetVelocity(_velocity);
		}
		inline FVector3 GetVelocity() const
		{
			return sound.GetVelocity();
		}

		inline void SetPosition(const FVector3& _position) const
		{
			sound.SetPosition(_position);
		}
		inline FVector3 GetPosition() const
		{
			return sound.GetPosition();
		}

		inline void SetPlayingOffset(Float _time) const
		{
			sound.SetPlayingOffset(_time);
		}
		inline Float GetPlayingOffset() const
		{
			return sound.GetPlayingOffset();
		}

		inline void SetPan(Float _pan) const
		{
			sound.SetPan(_pan);
		}
		inline Float GetPan() const
		{
			return sound.GetPan();
		}

		inline void SetMinGain(Float _gain) const
		{
			sound.SetMinGain(_gain);
		}
		inline Float GetMinGain() const
		{
			return sound.GetMinGain();
		}
		inline void SetMaxGain(Float _gain) const
		{
			sound.SetMaxGain(_gain);
		}
		inline Float GetMaxGain() const
		{
			return sound.GetMaxGain();
		}

		inline void SetMinDistance(Float _distance) const
		{
			sound.SetMinDistance(_distance);
		}
		inline Float GetMinDistance() const
		{
			return sound.GetMinDistance();
		}
		inline void SetMaxDistance(Float _distance) const
		{
			sound.SetMaxDistance(_distance);
		}
		inline Float GetMaxDistance() const
		{
			return sound.GetMaxDistance();
		}

		inline void SetDopplerFactor(Float _factor) const
		{
			sound.SetDopplerFactor(_factor);
		}
		inline Float GetDopplerFactor() const
		{
			return sound.GetDopplerFactor();
		}

		inline void SetDirectionalAttenuationFactor(Float _factor) const
		{
			sound.SetDirectionalAttenuationFactor(_factor);
		}
		inline Float GetDirectionalAttenuationFactor() const
		{
			return sound.GetDirectionalAttenuationFactor();
		}

		inline void SetCone(const sf::SoundSource::Cone& _cont)
		{
			return sound.SetCone(_cont);
		}
		inline sf::SoundSource::Cone GetCone() const
		{
			return sound.GetCone();
		}

		inline void SetDirection(const FVector3& _direction) const
		{
			sound.SetDirection(_direction);
		}
		inline FVector3 GetDirection() const
		{
			return sound.GetDirection();
		}
	};

}