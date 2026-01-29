#pragma once
#include "Sample.h"

namespace Krampus
{


	class Sound : public Sample
	{
		using SoundStatus = sf::Sound::Status;

		std::unique_ptr<sf::Sound> sound;
		std::shared_ptr<sf::SoundBuffer> buffer;

	public:
		inline virtual float GetVolume() const override
		{
			return sound->getVolume();
		}
		inline virtual void SetVolume(const float& _volume) override
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

		inline virtual bool IsLooping() const override
		{
			return sound->isLooping();
		}
		inline virtual void SetLoop(const bool& _loop) override
		{
			sound->setLooping(_loop);
		}

		inline virtual float GetPitch() const override
		{
			return sound->getPitch();
		}
		inline virtual void SetPitch(const float& _pitch) override
		{
			sound->setPitch(_pitch);
		}
		inline virtual void AddPitch(const float& _pitch) override
		{
			SetPitch(GetPitch() + _pitch);
		}

		inline virtual bool IsAvaliable() const override
		{
			return !IsPlaying();
		}
		inline virtual bool IsPlaying() const override
		{
			return sound->getStatus() == SoundStatus::Playing;
		}
		inline virtual bool IsPaused() const override
		{
			return sound->getStatus() == SoundStatus::Paused;
		}
		inline virtual bool IsStopped() const override
		{
			return sound->getStatus() == SoundStatus::Stopped;
		}

		inline virtual void SetAttenuation(float _attenuation)
		{
			sound->setAttenuation(_attenuation);
		}
		inline virtual float GetAttenuation() const
		{
			return sound->getAttenuation();
		}

		inline void SetSpatializationEnabled(bool _enable) const
		{
			sound->setSpatializationEnabled(_enable);
		}
		inline bool IsSpatializationEnabled() const
		{
			return sound->isSpatializationEnabled();
		}

		inline void SetRelativeToListener(bool _relative) const
		{
			sound->setRelativeToListener(_relative);
		}
		inline bool IsRelativeToListener() const
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

		inline void SetPlayingOffset(float _time) const
		{
			sound->setPlayingOffset(sf::seconds(_time));
		}
		inline float GetPlayingOffset() const
		{
			return sound->getPlayingOffset().asSeconds();
		}

		inline void SetPan(float _pan) const
		{
			sound->setPan(_pan);
		}
		inline float GetPan() const
		{
			return sound->getPan();
		}

		inline void SetMinGain(float _gain) const
		{
			sound->setMinGain(_gain);
		}
		inline float GetMinGain() const
		{
			return sound->getMinGain();
		}
		inline void SetMaxGain(float _gain) const
		{
			sound->setMaxGain(_gain);
		}
		inline float GetMaxGain() const
		{
			return sound->getMaxGain();
		}

		inline void SetMinDistance(float _distance) const
		{
			sound->setMinDistance(_distance);
		}
		inline float GetMinDistance() const
		{
			return sound->getMinDistance();
		}
		inline void SetMaxDistance(float _distance) const
		{
			sound->setMaxDistance(_distance);
		}
		inline float GetMaxDistance() const
		{
			return sound->getMaxDistance();
		}

		inline void SetDopplerFactor(float _factor) const
		{
			sound->setDopplerFactor(_factor);
		}
		inline float GetDopplerFactor() const
		{
			return sound->getDopplerFactor();
		}

		inline void SetDirectionalAttenuationFactor(float _factor) const
		{
			sound->setDirectionalAttenuationFactor(_factor);
		}
		inline float GetDirectionalAttenuationFactor() const
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


		inline Sound(std::shared_ptr<sf::SoundBuffer> _buffer)
		{
			buffer = std::move(_buffer);
			sound = std::make_unique<sf::Sound>(*buffer);
		}

		inline const std::shared_ptr<sf::SoundBuffer>& GetSharedBuffer() const
		{
			return buffer;
		}

	};

}