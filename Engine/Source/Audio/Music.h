#pragma once
#include "Sample.h"
#include "Utilities/Math/Vector3D.h"

namespace Krampus
{

	class Music : public Sample
	{
		using MusicStatus = sf::Music::Status;

		std::unique_ptr<sf::Music>	music;

	public:
		inline virtual Float GetVolume() const override
		{
			return music->getVolume();
		}
		inline virtual void SetVolume(const Float& _volume) override
		{
			return music->setVolume(_volume);
		}

		inline virtual Bool IsLooping() const override
		{
			return music->isLooping();
		}
		inline virtual void SetLoop(const Bool& _loop) override
		{
			music->setLooping(_loop);
		}
		inline virtual Float GetPitch() const override
		{
			return music->getPitch();
		}
		inline virtual void SetPitch(const Float& _pitch) override
		{
			music->setPitch(_pitch);
		}
		inline virtual void AddPitch(const Float& _pitch) override
		{
			SetPitch(GetPitch() + _pitch);
		}

		inline virtual Bool IsAvaliable() const override
		{
			return !IsPlaying();
		}
		inline Bool IsPlaying() const override
		{
			return music->getStatus() == MusicStatus::Playing;
		}
		inline Bool IsPaused() const override
		{
			return music->getStatus() == MusicStatus::Paused;
		}
		inline Bool IsStopped() const override
		{
			return music->getStatus() == MusicStatus::Stopped;
		}

		inline Float GetDuration() const
		{ 
			return music->getDuration().asSeconds();
		}

		inline virtual void SetAttenuation(Float _attenuation)
		{
			music->setAttenuation(_attenuation);
		}
		inline virtual Float GetAttenuation() const
		{
			return music->getAttenuation();
		}

		inline void SetSpatializationEnabled(Bool _enable) const
		{
			music->setSpatializationEnabled(_enable);
		}
		inline Bool IsSpatializationEnabled() const
		{
			return music->isSpatializationEnabled();
		}

		inline void SetRelativeToListener(Bool _relative) const
		{
			music->setRelativeToListener(_relative);
		}
		inline Bool IsRelativeToListener() const
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

		inline void SetPlayingOffset(Float _time) const
		{
			music->setPlayingOffset(sf::seconds(_time));
		}
		inline Float GetPlayingOffset() const
		{
			return music->getPlayingOffset().asSeconds();
		}

		inline void SetPan(Float _pan) const
		{
			music->setPan(_pan);
		}
		inline Float GetPan() const
		{
			return music->getPan();
		}

		inline void SetMinGain(Float _gain) const
		{
			music->setMinGain(_gain);
		}
		inline Float GetMinGain() const
		{
			return music->getMinGain();
		}
		inline void SetMaxGain(Float _gain) const
		{
			music->setMaxGain(_gain);
		}
		inline Float GetMaxGain() const
		{
			return music->getMaxGain();
		}

		inline void SetMinDistance(Float _distance) const
		{
			music->setMinDistance(_distance);
		}
		inline Float GetMinDistance() const
		{
			return music->getMinDistance();
		}
		inline void SetMaxDistance(Float _distance) const
		{
			music->setMaxDistance(_distance);
		}
		inline Float GetMaxDistance() const
		{
			return music->getMaxDistance();
		}

		inline void SetDopplerFactor(Float _factor) const
		{
			music->setDopplerFactor(_factor);
		}
		inline Float GetDopplerFactor() const
		{
			return music->getDopplerFactor();
		}

		inline void SetDirectionalAttenuationFactor(Float _factor) const
		{
			music->setDirectionalAttenuationFactor(_factor);
		}
		inline Float GetDirectionalAttenuationFactor() const
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
			: music(std::make_unique<sf::Music>()) 
		{
			SetVolume(50.0f);
			SetSpatializationEnabled(false);
		}

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

		inline Bool Open(const String& _path)
		{
			return music->openFromFile(_path.StdString());
		}
	};

}