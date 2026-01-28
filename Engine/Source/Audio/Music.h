#pragma once
#include "Sample.h"
#include "Utilities/Math/Vector2D.h"

namespace Krampus
{
	//TODO implement spacial sound

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