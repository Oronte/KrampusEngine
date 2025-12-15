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
		INLINE virtual float GetVolume() const override
		{
			return music->getVolume();
		}
		INLINE virtual void SetVolume(const float& _volume) override
		{
			return music->setVolume(_volume);
		}

		INLINE virtual bool IsLooping() const override
		{
			return music->isLooping();
		}
		INLINE virtual void SetLoop(const bool& _loop) override
		{
			music->setLooping(_loop);
		}
		INLINE virtual float GetPitch() const override
		{
			return music->getPitch();
		}
		INLINE virtual void SetPitch(const float& _pitch) override
		{
			music->setPitch(_pitch);
		}
		INLINE virtual void AddPitch(const float& _pitch) override
		{
			SetPitch(GetPitch() + _pitch);
		}

		INLINE virtual bool IsAvaliable() const override
		{
			return !IsPlaying();
		}
		INLINE bool IsPlaying() const override
		{
			return music->getStatus() == MusicStatus::Playing;
		}
		INLINE bool IsPaused() const override
		{
			return music->getStatus() == MusicStatus::Paused;
		}
		INLINE bool IsStopped() const override
		{
			return music->getStatus() == MusicStatus::Stopped;
		}

		INLINE float GetDuration() const 
		{ 
			return music->getDuration().asSeconds();
		}

		INLINE Music() 
			: music(std::make_unique<sf::Music>()) { }

		INLINE void Play()
		{
			music->play();
		}

		INLINE void Pause()
		{
			music->pause();
		}

		INLINE void Stop()
		{
			music->stop();
		}

		INLINE bool Open(const std::string& _path)
		{
			return music->openFromFile(_path);
		}
	};

}