#pragma once
#include "Sample.h"
#include "Utilities/Math/Vector3D.h"

namespace Krampus
{


	class Sound : public Sample
	{
		using SoundStatus = sf::Sound::Status;

		std::unique_ptr<sf::Sound>			sound;
		std::shared_ptr<sf::SoundBuffer>	buffer;

	public:
		inline virtual Float GetVolume() const override
		{
			return sound->getVolume();
		}
		inline virtual void SetVolume(const Float& _volume) override
		{
			sound->setVolume(_volume);
		}

		inline virtual void Play() override
		{
			sound->play();
		}
		inline virtual void Pause() override
		{
			sound->pause();
		}
		inline virtual void Stop() override
		{
			sound->stop();
		}

		inline virtual Bool IsLooping() const override
		{
			return sound->isLooping();
		}
		inline virtual void SetLoop(const Bool& _loop) override
		{
			sound->setLooping(_loop);
		}

		inline virtual Float GetPitch() const override
		{
			return sound->getPitch();
		}
		inline virtual void SetPitch(const Float& _pitch) override
		{
			sound->setPitch(_pitch);
		}
		inline virtual void AddPitch(const Float& _pitch) override
		{
			SetPitch(GetPitch() + _pitch);
		}

		inline virtual Bool IsAvaliable() const override
		{
			return !IsPlaying();
		}
		inline virtual Bool IsPlaying() const override
		{
			return sound->getStatus() == SoundStatus::Playing;
		}
		inline virtual Bool IsPaused() const override
		{
			return sound->getStatus() == SoundStatus::Paused;
		}
		inline virtual Bool IsStopped() const override
		{
			return sound->getStatus() == SoundStatus::Stopped;
		}

		inline virtual void SetAttenuation(Float _attenuation)
		{
			sound->setAttenuation(_attenuation);
		}
		inline virtual Float GetAttenuation() const
		{
			return sound->getAttenuation();
		}

		inline void SetSpatializationEnabled(Bool _enable) const
		{
			sound->setSpatializationEnabled(_enable);
		}
		inline Bool IsSpatializationEnabled() const
		{
			return sound->isSpatializationEnabled();
		}

		inline void SetRelativeToListener(Bool _relative) const
		{
			sound->setRelativeToListener(_relative);
		}
		inline Bool IsRelativeToListener() const
		{
			return sound->isRelativeToListener();
		}

		inline void SetVelocity(const FVector3& _velocity) const
		{
			sound->setVelocity(_velocity);
		}
		inline FVector3 GetVelocity() const
		{
			return sound->getVelocity();
		}

		inline void SetPosition(const FVector3& _position) const
		{
			sound->setPosition(_position);
		}
		inline FVector3 GetPosition() const
		{
			return sound->getPosition();
		}

		inline void SetPlayingOffset(Float _time) const
		{
			sound->setPlayingOffset(sf::seconds(_time));
		}
		inline Float GetPlayingOffset() const
		{
			return sound->getPlayingOffset().asSeconds();
		}

		inline void SetPan(Float _pan) const
		{
			sound->setPan(_pan);
		}
		inline Float GetPan() const
		{
			return sound->getPan();
		}

		inline void SetMinGain(Float _gain) const
		{
			sound->setMinGain(_gain);
		}
		inline Float GetMinGain() const
		{
			return sound->getMinGain();
		}
		inline void SetMaxGain(Float _gain) const
		{
			sound->setMaxGain(_gain);
		}
		inline Float GetMaxGain() const
		{
			return sound->getMaxGain();
		}

		inline void SetMinDistance(Float _distance) const
		{
			sound->setMinDistance(_distance);
		}
		inline Float GetMinDistance() const
		{
			return sound->getMinDistance();
		}
		inline void SetMaxDistance(Float _distance) const
		{
			sound->setMaxDistance(_distance);
		}
		inline Float GetMaxDistance() const
		{
			return sound->getMaxDistance();
		}

		inline void SetDopplerFactor(Float _factor) const
		{
			sound->setDopplerFactor(_factor);
		}
		inline Float GetDopplerFactor() const
		{
			return sound->getDopplerFactor();
		}

		inline void SetDirectionalAttenuationFactor(Float _factor) const
		{
			sound->setDirectionalAttenuationFactor(_factor);
		}
		inline Float GetDirectionalAttenuationFactor() const
		{
			return sound->getDirectionalAttenuationFactor();
		}

		inline void SetCone(const sf::SoundSource::Cone& _cont)
		{
			return sound->setCone(_cont);
		}
		inline sf::SoundSource::Cone GetCone() const
		{
			return sound->getCone();
		}

		inline void SetDirection(const FVector3& _direction) const
		{
			sound->setDirection(_direction);
		}
		inline FVector3 GetDirection() const
		{
			return sound->getDirection();
		}


		Sound() = default;
		inline Sound(std::shared_ptr<sf::SoundBuffer> _buffer)
		{
			buffer = std::move(_buffer);
			sound = std::make_unique<sf::Sound>(*buffer);
			SetVolume(50.0f);
			SetSpatializationEnabled(false);
		}

		inline const std::shared_ptr<sf::SoundBuffer>& GetSharedBuffer() const
		{
			return buffer;
		}

	};

}