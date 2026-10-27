#pragma once
#include "Utilities/Macro.h"

namespace Krampus
{

	/// <summary>
	/// Abstract interface representing an audio playback sample (sound or music).
	/// </summary>
	class Sample
	{
	public:
		/// <summary>
		/// Default constructor for Sample.
		/// </summary>
		Sample() = default;

		/// <summary>
		/// Virtual destructor for Sample.
		/// </summary>
		virtual ~Sample() = default;

		/// <summary>
		/// Gets the volume of the audio sample in the range [0, 100].
		/// </summary>
		/// <returns>Current volume level.</returns>
		NO_DISCARD virtual Float GetVolume() const = 0;

		/// <summary>
		/// Sets the volume of the audio sample in the range [0, 100].
		/// </summary>
		/// <param name="_volume">The new volume level.</param>
		virtual void SetVolume(const Float& _volume) = 0;

		/// <summary>
		/// Starts or resumes playing the audio sample.
		/// </summary>
		virtual void Play() = 0;

		/// <summary>
		/// Pauses playback of the audio sample.
		/// </summary>
		virtual void Pause() = 0;

		/// <summary>
		/// Stops playback and resets the playback position to the beginning.
		/// </summary>
		virtual void Stop() = 0;

		/// <summary>
		/// Checks whether the audio sample is set to loop indefinitely.
		/// </summary>
		/// <returns>True if looping is enabled, false otherwise.</returns>
		NO_DISCARD virtual Bool IsLooping() const = 0;

		/// <summary>
		/// Sets whether the audio sample should loop continuously.
		/// </summary>
		/// <param name="_loop">True to enable looping, false to disable.</param>
		virtual void SetLoop(const Bool& _loop) = 0;

		/// <summary>
		/// Gets the pitch multiplier of the audio sample.
		/// </summary>
		/// <returns>Current pitch multiplier (1.0 is default).</returns>
		NO_DISCARD virtual Float GetPitch() const = 0;

		/// <summary>
		/// Sets the pitch multiplier of the audio sample.
		/// </summary>
		/// <param name="_pitch">The new pitch multiplier.</param>
		virtual void SetPitch(const Float& _pitch) = 0;

		/// <summary>
		/// Adds an offset to the current pitch multiplier.
		/// </summary>
		/// <param name="_pitch">The pitch delta to add.</param>
		virtual void AddPitch(const Float& _pitch) = 0;

		/// <summary>
		/// Checks if the audio sample is available to be played (i.e., not currently playing).
		/// </summary>
		/// <returns>True if available, false if currently playing.</returns>
		NO_DISCARD virtual Bool IsAvaliable() const = 0;

		/// <summary>
		/// Checks whether the audio sample is currently playing.
		/// </summary>
		/// <returns>True if playing, false otherwise.</returns>
		NO_DISCARD virtual Bool IsPlaying() const = 0;

		/// <summary>
		/// Checks whether the audio sample is currently paused.
		/// </summary>
		/// <returns>True if paused, false otherwise.</returns>
		NO_DISCARD virtual Bool IsPaused() const = 0;

		/// <summary>
		/// Checks whether the audio sample is currently stopped.
		/// </summary>
		/// <returns>True if stopped, false otherwise.</returns>
		NO_DISCARD virtual Bool IsStopped() const = 0;
	};

}
