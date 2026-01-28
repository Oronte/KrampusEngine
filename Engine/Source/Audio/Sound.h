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