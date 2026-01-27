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
		inlin virtual float GetVolume() const override
		{
			return music->getVolume();
		}
		inlin virtual void SetVolume(const float& _volume) override
		{
			return music->setVolume(_volume);
		}

		inlin virtual bool IsLooping() const override
		{
			return music->isLooping();
		}
		inlin virtual void SetLoop(const bool& _loop) override
		{
			music->setLooping(_loop);
		}
		inlin virtual float GetPitch() const override
		{
			return music->getPitch();
		}
		inlin virtual void SetPitch(const float& _pitch) override
		{
			music->setPitch(_pitch);
		}
		inlin virtual void AddPitch(const float& _pitch) override
		{
			SetPitch(GetPitch() + _pitch);
		}

		inlin virtual bool IsAvaliable() const override
		{
			return !IsPlaying();
		}
		inlin bool IsPlaying() const override
		{
			return music->getStatus() == MusicStatus::Playing;
		}
		inlin bool IsPaused() const override
		{
			return music->getStatus() == MusicStatus::Paused;
		}
		inlin bool IsStopped() const override
		{
			return music->getStatus() == MusicStatus::Stopped;
		}

		inlin float GetDuration() const 
		{ 
			return music->getDuration().asSeconds();
		}

		inlin Music() 
			: music(std::make_unique<sf::Music>()) { }

		inlin void Play()
		{
			music->play();
		}

		inlin void Pause()
		{
			music->pause();
		}

		inlin void Stop()
		{
			music->stop();
		}

		inlin bool Open(const std::string& _path)
		{
			return music->openFromFile(_path);
		}
	};

}