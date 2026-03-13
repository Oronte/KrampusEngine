#pragma once
#include "Utilities/Macro.h"

namespace Krampus
{

	class Sample
	{
	public:
        Sample() = default;

        virtual Float GetVolume() const = 0;
        virtual void SetVolume(const Float& _volume) = 0;

        virtual void Play() = 0;
        virtual void Pause() = 0;
        virtual void Stop() = 0;

        virtual Bool IsLooping() const = 0;
        virtual void SetLoop(const Bool& _loop) = 0;

        virtual Float GetPitch() const = 0;
        virtual void SetPitch(const Float& _pitch) = 0;
        virtual void AddPitch(const Float& _pitch) = 0;

        virtual Bool IsAvaliable() const = 0; // Can be played
        virtual Bool IsPlaying() const = 0;
        virtual Bool IsPaused() const = 0;
        virtual Bool IsStopped() const = 0;
	};

}

