#pragma once
#include "Utilities/Macro.h"

namespace Krampus
{

	class Sample
	{
	public:
		Sample() = default;

        virtual float GetVolume() const = 0;
        virtual void SetVolume(const float& _volume) = 0;

        virtual void Play() = 0;
        virtual void Pause() = 0;
        virtual void Stop() = 0;

        virtual bool IsLooping() const = 0;
        virtual void SetLoop(const bool& _loop) = 0;

        virtual float GetPitch() const = 0;
        virtual void SetPitch(const float& _pitch) = 0;
        virtual void AddPitch(const float& _pitch) = 0;

        virtual bool IsAvaliable() const = 0; // Can be played
        virtual bool IsPlaying() const = 0;
        virtual bool IsPaused() const = 0;
        virtual bool IsStopped() const = 0;
	};

}

