#pragma once
#include "Sample.h"
#include "Utilities/Math/Vector2D.h"

namespace Krampus
{

	class Music : public Sample
	{
		using MusicStatus = sf::Music::Status;

		std::unique_ptr<sf::Music> music;

	public:
		inline virtual float GetVolume() const override
		{
			return music->getVolume();
		}
		inline virtual void SetVolume(const float& _volume) override
		{
			return music->setVolume(_volume);
		}

		inline virtual bool IsLooping() const override
		{
			return music->isLooping();
		}
		inline virtual void SetLoop(const bool& _loop) override
		{
			music->setLooping(_loop);
		}
		inline virtual float GetPitch() const override
		{
			return music->getPitch();
		}
		inline virtual void SetPitch(const float& _pitch) override
		{
			music->setPitch(_pitch);
		}
		inline virtual void AddPitch(const float& _pitch) override
		{
			SetPitch(GetPitch() + _pitch);
		}

		inline virtual bool IsAvaliable() const override
		{
			return !IsPlaying();
		}
		inline bool IsPlaying() const override
		{
			return music->getStatus() == MusicStatus::Playing;
		}
		inline bool IsPaused() const override
		{
			return music->getStatus() == MusicStatus::Paused;
		}
		inline bool IsStopped() const override
		{
			return music->getStatus() == MusicStatus::Stopped;
		}

		inline float GetDuration() const
		{ 
			return music->getDuration().asSeconds();
		}

		inline virtual void SetAttenuation(float _attenuation)
		{
			music->setAttenuation(_attenuation);
		}
		inline virtual float GetAttenuation() const
		{
			return music->getAttenuation();
		}

		inline void SetSpatializationEnabled(bool _enable) const
		{
			music->setSpatializationEnabled(_enable);
		}
		inline bool IsSpatializationEnabled() const
		{
			return music->isSpatializationEnabled();
		}

		inline void SetRelativeToListener(bool _relative) const
		{
			music->setRelativeToListener(_relative);
		}
		inline bool IsRelativeToListener() const
		{
			return music->isRelativeToListener();
		}

		inline void SetVelocity(const FVector3& _velocity) const
		{
			music->setVelocity(_velocity);
		}
		inline FVector3 GetVelocity() const
		{
			return music->getVelocity();
		}

		inline void SetPosition(const FVector3& _position) const
		{
			music->setPosition(_position);
		}
		inline FVector3 GetPosition() const
		{
			return music->getPosition();
		}

		inline void SetPlayingOffset(float _time) const
		{
			music->setPlayingOffset(sf::seconds(_time));
		}
		inline float GetPlayingOffset() const
		{
			return music->getPlayingOffset().asSeconds();
		}

		inline void SetPan(float _pan) const
		{
			music->setPan(_pan);
		}
		inline float GetPan() const
		{
			return music->getPan();
		}

		inline void SetMinGain(float _gain) const
		{
			music->setMinGain(_gain);
		}
		inline float GetMinGain() const
		{
			return music->getMinGain();
		}
		inline void SetMaxGain(float _gain) const
		{
			music->setMaxGain(_gain);
		}
		inline float GetMaxGain() const
		{
			return music->getMaxGain();
		}

		inline void SetMinDistance(float _distance) const
		{
			music->setMinDistance(_distance);
		}
		inline float GetMinDistance() const
		{
			return music->getMinDistance();
		}
		inline void SetMaxDistance(float _distance) const
		{
			music->setMaxDistance(_distance);
		}
		inline float GetMaxDistance() const
		{
			return music->getMaxDistance();
		}

		inline void SetDopplerFactor(float _factor) const
		{
			music->setDopplerFactor(_factor);
		}
		inline float GetDopplerFactor() const
		{
			return music->getDopplerFactor();
		}

		inline void SetDirectionalAttenuationFactor(float _factor) const
		{
			music->setDirectionalAttenuationFactor(_factor);
		}
		inline float GetDirectionalAttenuationFactor() const
		{
			return music->getDirectionalAttenuationFactor();
		}

		inline void SetCone(const sf::SoundSource::Cone& _cont)
		{
			return music->setCone(_cont);
		}
		inline sf::SoundSource::Cone GetCone() const
		{
			return music->getCone();
		}

		inline void SetDirection(const FVector3& _direction) const
		{
			music->setDirection(_direction);
		}
		inline FVector3 GetDirection() const
		{
			return music->getDirection();
		}

		inline Music()
			: music(std::make_unique<sf::Music>()) { }

		inline void Play()
		{
			music->play();
		}

		inline void Pause()
		{
			music->pause();
		}

		inline void Stop()
		{
			music->stop();
		}

		inline bool Open(const std::string& _path)
		{
			return music->openFromFile(_path);
		}
	};

}