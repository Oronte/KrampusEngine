#pragma once
#include "Sample.h"
#include "Utilities/Math/Vector3D.h"

namespace Krampus
{

	/// <summary>
	/// Represents streamed background audio or music tracks.
	/// </summary>
	class Music : public Sample
	{
		using MusicStatus = sf::Music::Status;

		std::unique_ptr<sf::Music>	music;

	public:
		/// <summary>
		/// Gets the playback volume of the music in the range [0, 100].
		/// </summary>
		/// <returns>Current volume level.</returns>
		inline virtual Float GetVolume() const override
		{
			return music->getVolume();
		}

		/// <summary>
		/// Sets the playback volume of the music in the range [0, 100].
		/// </summary>
		/// <param name="_volume">The new volume level.</param>
		inline virtual void SetVolume(const Float& _volume) override
		{
			return music->setVolume(_volume);
		}

		/// <summary>
		/// Checks whether the music loops when finished.
		/// </summary>
		/// <returns>True if loop is enabled, false otherwise.</returns>
		inline virtual Bool IsLooping() const override
		{
			return music->isLooping();
		}

		/// <summary>
		/// Enables or disables continuous looping of the music track.
		/// </summary>
		/// <param name="_loop">True to enable looping, false to disable.</param>
		inline virtual void SetLoop(const Bool& _loop) override
		{
			music->setLooping(_loop);
		}

		/// <summary>
		/// Gets the pitch multiplier of the music track.
		/// </summary>
		/// <returns>Pitch multiplier value.</returns>
		inline virtual Float GetPitch() const override
		{
			return music->getPitch();
		}

		/// <summary>
		/// Sets the pitch multiplier of the music track.
		/// </summary>
		/// <param name="_pitch">New pitch multiplier.</param>
		inline virtual void SetPitch(const Float& _pitch) override
		{
			music->setPitch(_pitch);
		}

		/// <summary>
		/// Adds an offset to the current pitch multiplier.
		/// </summary>
		/// <param name="_pitch">Pitch offset to add.</param>
		inline virtual void AddPitch(const Float& _pitch) override
		{
			SetPitch(GetPitch() + _pitch);
		}

		/// <summary>
		/// Checks whether the music track is available to be played.
		/// </summary>
		/// <returns>True if not playing, false if playing.</returns>
		inline virtual Bool IsAvaliable() const override
		{
			return !IsPlaying();
		}

		/// <summary>
		/// Checks whether the music track is currently playing.
		/// </summary>
		/// <returns>True if playing, false otherwise.</returns>
		inline Bool IsPlaying() const override
		{
			return music->getStatus() == MusicStatus::Playing;
		}

		/// <summary>
		/// Checks whether the music track is currently paused.
		/// </summary>
		/// <returns>True if paused, false otherwise.</returns>
		inline Bool IsPaused() const override
		{
			return music->getStatus() == MusicStatus::Paused;
		}

		/// <summary>
		/// Checks whether the music track is currently stopped.
		/// </summary>
		/// <returns>True if stopped, false otherwise.</returns>
		inline Bool IsStopped() const override
		{
			return music->getStatus() == MusicStatus::Stopped;
		}

		/// <summary>
		/// Gets the total duration of the music track in seconds.
		/// </summary>
		/// <returns>Duration in seconds.</returns>
		inline Float GetDuration() const
		{ 
			return music->getDuration().asSeconds();
		}

		/// <summary>
		/// Sets the distance attenuation factor for 3D positional audio.
		/// </summary>
		/// <param name="_attenuation">Attenuation factor.</param>
		inline virtual void SetAttenuation(Float _attenuation)
		{
			music->setAttenuation(_attenuation);
		}

		/// <summary>
		/// Gets the 3D distance attenuation factor.
		/// </summary>
		/// <returns>Attenuation factor.</returns>
		inline virtual Float GetAttenuation() const
		{
			return music->getAttenuation();
		}

		/// <summary>
		/// Enables or disables 3D spatialization for this music stream.
		/// </summary>
		/// <param name="_enable">True to enable spatialization, false to disable.</param>
		inline void SetSpatializationEnabled(Bool _enable) const
		{
			music->setSpatializationEnabled(_enable);
		}

		/// <summary>
		/// Checks whether 3D spatialization is enabled for this music stream.
		/// </summary>
		/// <returns>True if spatialized, false otherwise.</returns>
		inline Bool IsSpatializationEnabled() const
		{
			return music->isSpatializationEnabled();
		}

		/// <summary>
		/// Sets whether the music position is relative to the audio listener.
		/// </summary>
		/// <param name="_relative">True for listener-relative positioning, false for absolute.</param>
		inline void SetRelativeToListener(Bool _relative) const
		{
			music->setRelativeToListener(_relative);
		}

		/// <summary>
		/// Checks whether the music position is relative to the audio listener.
		/// </summary>
		/// <returns>True if relative to listener, false otherwise.</returns>
		inline Bool IsRelativeToListener() const
		{
			return music->isRelativeToListener();
		}

		/// <summary>
		/// Sets the 3D velocity vector of the music source (used for Doppler calculations).
		/// </summary>
		/// <param name="_velocity">Velocity 3D vector.</param>
		inline void SetVelocity(const FVector3& _velocity) const
		{
			music->setVelocity(_velocity);
		}

		/// <summary>
		/// Gets the 3D velocity vector of the music source.
		/// </summary>
		/// <returns>Velocity 3D vector.</returns>
		inline FVector3 GetVelocity() const
		{
			return music->getVelocity();
		}

		/// <summary>
		/// Sets the 3D world position of the music source.
		/// </summary>
		/// <param name="_position">3D position coordinates.</param>
		inline void SetPosition(const FVector3& _position) const
		{
			music->setPosition(_position);
		}

		/// <summary>
		/// Gets the 3D world position of the music source.
		/// </summary>
		/// <returns>3D position coordinates.</returns>
		inline FVector3 GetPosition() const
		{
			return music->getPosition();
		}

		/// <summary>
		/// Sets the current playback offset in seconds.
		/// </summary>
		/// <param name="_time">Time offset in seconds.</param>
		inline void SetPlayingOffset(Float _time) const
		{
			music->setPlayingOffset(sf::seconds(_time));
		}

		/// <summary>
		/// Gets the current playback offset in seconds.
		/// </summary>
		/// <returns>Playback offset in seconds.</returns>
		inline Float GetPlayingOffset() const
		{
			return music->getPlayingOffset().asSeconds();
		}

		/// <summary>
		/// Sets the stereo panning value in the range [-1.0 (left), 1.0 (right)].
		/// </summary>
		/// <param name="_pan">Pan value.</param>
		inline void SetPan(Float _pan) const
		{
			music->setPan(_pan);
		}

		/// <summary>
		/// Gets the stereo panning value.
		/// </summary>
		/// <returns>Pan value.</returns>
		inline Float GetPan() const
		{
			return music->getPan();
		}

		/// <summary>
		/// Sets the minimum gain (volume floor) when distance increases.
		/// </summary>
		/// <param name="_gain">Minimum gain factor.</param>
		inline void SetMinGain(Float _gain) const
		{
			music->setMinGain(_gain);
		}

		/// <summary>
		/// Gets the minimum gain factor.
		/// </summary>
		/// <returns>Minimum gain factor.</returns>
		inline Float GetMinGain() const
		{
			return music->getMinGain();
		}

		/// <summary>
		/// Sets the maximum gain (volume ceiling).
		/// </summary>
		/// <param name="_gain">Maximum gain factor.</param>
		inline void SetMaxGain(Float _gain) const
		{
			music->setMaxGain(_gain);
		}

		/// <summary>
		/// Gets the maximum gain factor.
		/// </summary>
		/// <returns>Maximum gain factor.</returns>
		inline Float GetMaxGain() const
		{
			return music->getMaxGain();
		}

		/// <summary>
		/// Sets the minimum distance at which distance attenuation begins.
		/// </summary>
		/// <param name="_distance">Minimum distance value.</param>
		inline void SetMinDistance(Float _distance) const
		{
			music->setMinDistance(_distance);
		}

		/// <summary>
		/// Gets the minimum distance at which distance attenuation begins.
		/// </summary>
		/// <returns>Minimum distance value.</returns>
		inline Float GetMinDistance() const
		{
			return music->getMinDistance();
		}

		/// <summary>
		/// Sets the maximum distance beyond which music is no longer attenuated.
		/// </summary>
		/// <param name="_distance">Maximum distance value.</param>
		inline void SetMaxDistance(Float _distance) const
		{
			music->setMaxDistance(_distance);
		}

		/// <summary>
		/// Gets the maximum distance for attenuation.
		/// </summary>
		/// <returns>Maximum distance value.</returns>
		inline Float GetMaxDistance() const
		{
			return music->getMaxDistance();
		}

		/// <summary>
		/// Sets the Doppler effect scaling factor.
		/// </summary>
		/// <param name="_factor">Doppler scaling factor.</param>
		inline void SetDopplerFactor(Float _factor) const
		{
			music->setDopplerFactor(_factor);
		}

		/// <summary>
		/// Gets the Doppler effect scaling factor.
		/// </summary>
		/// <returns>Doppler scaling factor.</returns>
		inline Float GetDopplerFactor() const
		{
			return music->getDopplerFactor();
		}

		/// <summary>
		/// Sets the directional attenuation factor for sound cones.
		/// </summary>
		/// <param name="_factor">Directional attenuation factor.</param>
		inline void SetDirectionalAttenuationFactor(Float _factor) const
		{
			music->setDirectionalAttenuationFactor(_factor);
		}

		/// <summary>
		/// Gets the directional attenuation factor.
		/// </summary>
		/// <returns>Directional attenuation factor.</returns>
		inline Float GetDirectionalAttenuationFactor() const
		{
			return music->getDirectionalAttenuationFactor();
		}

		/// <summary>
		/// Sets the directional emission cone parameters for this music stream.
		/// </summary>
		/// <param name="_cone">Cone specification (inner angle, outer angle, outer gain).</param>
		inline void SetCone(const sf::SoundSource::Cone& _cone)
		{
			return music->setCone(_cone);
		}

		/// <summary>
		/// Gets the directional emission cone parameters.
		/// </summary>
		/// <returns>Current emission cone.</returns>
		inline sf::SoundSource::Cone GetCone() const
		{
			return music->getCone();
		}

		/// <summary>
		/// Sets the 3D direction vector that the music source is pointing towards.
		/// </summary>
		/// <param name="_direction">Direction 3D vector.</param>
		inline void SetDirection(const FVector3& _direction) const
		{
			music->setDirection(_direction);
		}

		/// <summary>
		/// Gets the 3D direction vector of the music source.
		/// </summary>
		/// <returns>Direction 3D vector.</returns>
		inline FVector3 GetDirection() const
		{
			return music->getDirection();
		}

		/// <summary>
		/// Constructs a Music instance with default volume (50%) and spatialization disabled.
		/// </summary>
		inline Music()
			: music(std::make_unique<sf::Music>()) 
		{
			SetVolume(50.0f);
			SetSpatializationEnabled(false);
		}

		/// <summary>
		/// Starts or resumes playing the music track.
		/// </summary>
		inline void Play()
		{
			music->play();
		}

		/// <summary>
		/// Pauses playback of the music track.
		/// </summary>
		inline void Pause()
		{
			music->pause();
		}

		/// <summary>
		/// Stops playback of the music track and rewinds to the beginning.
		/// </summary>
		inline void Stop()
		{
			music->stop();
		}

		/// <summary>
		/// Opens an audio music stream from the specified file path.
		/// </summary>
		/// <param name="_path">File path to the audio file.</param>
		/// <returns>True if the file was successfully opened, false otherwise.</returns>
		inline Bool Open(const String& _path)
		{
			return music->openFromFile(_path.StdString());
		}
	};

}
