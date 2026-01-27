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
		inlin virtual float GetVolume() const override
		{
			return sound->getVolume();
		}
		inlin virtual void SetVolume(const float& _volume) override
		{
			sound->setVolume(_volume);
		}

		inlin virtual void Play() override
		{
			sound->play();
		}
		inlin virtual void Pause() override
		{
			sound->pause();
		}
		inlin virtual void Stop() override
		{
			sound->stop();
		}

		inlin virtual bool IsLooping() const override
		{
			return sound->isLooping();
		}
		inlin virtual void SetLoop(const bool& _loop) override
		{
			sound->setLooping(_loop);
		}

		inlin virtual float GetPitch() const override
		{
			return sound->getPitch();
		}
		inlin virtual void SetPitch(const float& _pitch) override
		{
			sound->setPitch(_pitch);
		}
		inlin virtual void AddPitch(const float& _pitch) override
		{
			SetPitch(GetPitch() + _pitch);
		}

		inlin virtual bool IsAvaliable() const override
		{
			return !IsPlaying();
		}
		inlin virtual bool IsPlaying() const override
		{
			return sound->getStatus() == SoundStatus::Playing;
		}
		inlin virtual bool IsPaused() const override
		{
			return sound->getStatus() == SoundStatus::Paused;
		}
		inlin virtual bool IsStopped() const override
		{
			return sound->getStatus() == SoundStatus::Stopped;
		}

		inlin Sound(std::shared_ptr<sf::SoundBuffer> _buffer)
		{
			buffer = std::move(_buffer);
			sound = std::make_unique<sf::Sound>(*buffer);
		}

		inlin const std::shared_ptr<sf::SoundBuffer>& GetSharedBuffer() const
		{
			return buffer;
		}

	};

}