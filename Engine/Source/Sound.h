#pragma once
#include "Sample.h"

namespace Krampus
{
	//TODO implement spacial sound

	class Sound : public Sample
	{
		using SoundStatus = sf::Sound::Status;

		std::unique_ptr<sf::Sound> sound;
		std::shared_ptr<sf::SoundBuffer> buffer;

	public:
		INLINE virtual float GetVolume() const override
		{
			return sound->getVolume();
		}
		INLINE virtual void SetVolume(const float& _volume) override
		{
			sound->setVolume(_volume);
		}

		INLINE virtual void Play() override
		{
			sound->play();
		}
		INLINE virtual void Pause() override
		{
			sound->pause();
		}
		INLINE virtual void Stop() override
		{
			sound->stop();
		}

		INLINE virtual bool IsLooping() const override
		{
			return sound->isLooping();
		}
		INLINE virtual void SetLoop(const bool& _loop) override
		{
			sound->setLooping(_loop);
		}

		INLINE virtual float GetPitch() const override
		{
			return sound->getPitch();
		}
		INLINE virtual void SetPitch(const float& _pitch) override
		{
			sound->setPitch(_pitch);
		}
		INLINE virtual void AddPitch(const float& _pitch) override
		{
			SetPitch(GetPitch() + _pitch);
		}

		INLINE virtual bool IsAvaliable() const override
		{
			return !IsPlaying();
		}
		INLINE virtual bool IsPlaying() const override
		{
			return sound->getStatus() == SoundStatus::Playing;
		}
		INLINE virtual bool IsPaused() const override
		{
			return sound->getStatus() == SoundStatus::Paused;
		}
		INLINE virtual bool IsStopped() const override
		{
			return sound->getStatus() == SoundStatus::Stopped;
		}

		INLINE Sound(std::shared_ptr<sf::SoundBuffer> _buffer)
		{
			buffer = std::move(_buffer);
			sound = std::make_unique<sf::Sound>(*buffer);
		}

		INLINE const std::shared_ptr<sf::SoundBuffer>& GetSharedBuffer() const
		{
			return buffer;
		}

	};

}