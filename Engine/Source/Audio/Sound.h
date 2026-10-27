#pragma once
#include "Sample.h"
#include "Utilities/Math/Vector3D.h"

namespace Krampus
{

	/// <summary>
	/// Represents a playable, localized sound effect loaded from a sound buffer.
	/// </summary>
	class Sound : public Sample
	{
		using SoundStatus = sf::Sound::Status;

		std::unique_ptr<sf::Sound>			sound;
		std::shared_ptr<sf::SoundBuffer>	buffer;

	public:
		/// <summary>
		/// Gets the volume of the sound in the range [0, 100].
		/// </summary>
		/// <returns>Current volume level.</returns>
		inline virtual Float GetVolume() const override
		{
			return sound->getVolume();
		}

		/// <summary>
		/// Sets the volume of the sound in the range [0, 100].
		/// </summary>
		/// <param name="_volume">The new volume level.</param>
		inline virtual void SetVolume(const Float& _volume) override
		{
			sound->setVolume(_volume);
		}

		/// <summary>
		/// Starts or resumes playback of the sound.
		/// </summary>
		inline virtual void Play() override
		{
			sound->play();
		}

		/// <summary>
		/// Pauses playback of the sound.
		/// </summary>
		inline virtual void Pause() override
		{
			sound->pause();
		}

		/// <summary>
		/// Stops playback of the sound and resets position to the beginning.
		/// </summary>
		inline virtual void Stop() override
		{
			sound->stop();
		}

		/// <summary>
		/// Checks whether the sound loops when finished.
		/// </summary>
		/// <returns>True if loop is enabled, false otherwise.</returns>
		inline virtual Bool IsLooping() const override
		{
			return sound->isLooping();
		}

		/// <summary>
		/// Enables or disables continuous looping of the sound.
		/// </summary>
		/// <param name="_loop">True to enable looping, false to disable.</param>
		inline virtual void SetLoop(const Bool& _loop) override
		{
			sound->setLooping(_loop);
		}

		/// <summary>
		/// Gets the pitch multiplier of the sound.
		/// </summary>
		/// <returns>Pitch multiplier value.</returns>
		inline virtual Float GetPitch() const override
		{
			return sound->getPitch();
		}

		/// <summary>
		/// Sets the pitch multiplier of the sound.
		/// </summary>
		/// <param name="_pitch">New pitch multiplier.</param>
		inline virtual void SetPitch(const Float& _pitch) override
		{
			sound->setPitch(_pitch);
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
		/// Checks whether the sound is currently available to be played.
		/// </summary>
		/// <returns>True if not playing, false if playing.</returns>
		inline virtual Bool IsAvaliable() const override
		{
			return !IsPlaying();
		}

		/// <summary>
		/// Checks whether the sound is currently playing.
		/// </summary>
		/// <returns>True if playing, false otherwise.</returns>
		inline virtual Bool IsPlaying() const override
		{
			return sound->getStatus() == SoundStatus::Playing;
		}

		/// <summary>
		/// Checks whether the sound is currently paused.
		/// </summary>
		/// <returns>True if paused, false otherwise.</returns>
		inline virtual Bool IsPaused() const override
		{
			return sound->getStatus() == SoundStatus::Paused;
		}

		/// <summary>
		/// Checks whether the sound is currently stopped.
		/// </summary>
		/// <returns>True if stopped, false otherwise.</returns>
		inline virtual Bool IsStopped() const override
		{
			return sound->getStatus() == SoundStatus::Stopped;
		}

		/// <summary>
		/// Sets the attenuation factor for 3D positional distance falloff.
		/// </summary>
		/// <param name="_attenuation">Attenuation factor.</param>
		inline virtual void SetAttenuation(Float _attenuation)
		{
			sound->setAttenuation(_attenuation);
		}

		/// <summary>
		/// Gets the 3D distance attenuation factor.
		/// </summary>
		/// <returns>Attenuation factor.</returns>
		inline virtual Float GetAttenuation() const
		{
			return sound->getAttenuation();
		}

		/// <summary>
		/// Enables or disables 3D spatialization for this sound.
		/// </summary>
		/// <param name="_enable">True to enable spatialization, false to disable.</param>
		inline void SetSpatializationEnabled(Bool _enable) const
		{
			sound->setSpatializationEnabled(_enable);
		}

		/// <summary>
		/// Checks whether 3D spatialization is enabled for this sound.
		/// </summary>
		/// <returns>True if spatialized, false otherwise.</returns>
		inline Bool IsSpatializationEnabled() const
		{
			return sound->isSpatializationEnabled();
		}

		/// <summary>
		/// Sets whether the sound's position is relative to the audio listener.
		/// </summary>
		/// <param name="_relative">True for listener-relative positioning, false for absolute.</param>
		inline void SetRelativeToListener(Bool _relative) const
		{
			sound->setRelativeToListener(_relative);
		}

		/// <summary>
		/// Checks whether the sound's position is relative to the audio listener.
		/// </summary>
		/// <returns>True if relative to listener, false otherwise.</returns>
		inline Bool IsRelativeToListener() const
		{
			return sound->isRelativeToListener();
		}

		/// <summary>
		/// Sets the 3D velocity vector of the sound (used for Doppler calculations).
		/// </summary>
		/// <param name="_velocity">Velocity 3D vector.</param>
		inline void SetVelocity(const FVector3& _velocity) const
		{
			sound->setVelocity(_velocity);
		}

		/// <summary>
		/// Gets the 3D velocity vector of the sound.
		/// </summary>
		/// <returns>Velocity 3D vector.</returns>
		inline FVector3 GetVelocity() const
		{
			return sound->getVelocity();
		}

		/// <summary>
		/// Sets the 3D world position of the sound.
		/// </summary>
		/// <param name="_position">3D position coordinates.</param>
		inline void SetPosition(const FVector3& _position) const
		{
			sound->setPosition(_position);
		}

		/// <summary>
		/// Gets the 3D world position of the sound.
		/// </summary>
		/// <returns>3D position coordinates.</returns>
		inline FVector3 GetPosition() const
		{
			return sound->getPosition();
		}

		/// <summary>
		/// Sets the current playback offset in seconds.
		/// </summary>
		/// <param name="_time">Time offset in seconds.</param>
		inline void SetPlayingOffset(Float _time) const
		{
			sound->setPlayingOffset(sf::seconds(_time));
		}

		/// <summary>
		/// Gets the current playback offset in seconds.
		/// </summary>
		/// <returns>Playback offset in seconds.</returns>
		inline Float GetPlayingOffset() const
		{
			return sound->getPlayingOffset().asSeconds();
		}

		/// <summary>
		/// Sets the stereo panning value in the range [-1.0 (left), 1.0 (right)].
		/// </summary>
		/// <param name="_pan">Pan value.</param>
		inline void SetPan(Float _pan) const
		{
			sound->setPan(_pan);
		}

		/// <summary>
		/// Gets the stereo panning value.
		/// </summary>
		/// <returns>Pan value.</returns>
		inline Float GetPan() const
		{
			return sound->getPan();
		}

		/// <summary>
		/// Sets the minimum gain (volume floor) when distance increases.
		/// </summary>
		/// <param name="_gain">Minimum gain factor.</param>
		inline void SetMinGain(Float _gain) const
		{
			sound->setMinGain(_gain);
		}

		/// <summary>
		/// Gets the minimum gain factor.
		/// </summary>
		/// <returns>Minimum gain factor.</returns>
		inline Float GetMinGain() const
		{
			return sound->getMinGain();
		}

		/// <summary>
		/// Sets the maximum gain (volume ceiling).
		/// </summary>
		/// <param name="_gain">Maximum gain factor.</param>
		inline void SetMaxGain(Float _gain) const
		{
			sound->setMaxGain(_gain);
		}

		/// <summary>
		/// Gets the maximum gain factor.
		/// </summary>
		/// <returns>Maximum gain factor.</returns>
		inline Float GetMaxGain() const
		{
			return sound->getMaxGain();
		}

		/// <summary>
		/// Sets the minimum distance at which distance attenuation begins.
		/// </summary>
		/// <param name="_distance">Minimum distance value.</param>
		inline void SetMinDistance(Float _distance) const
		{
			sound->setMinDistance(_distance);
		}

		/// <summary>
		/// Gets the minimum distance at which distance attenuation begins.
		/// </summary>
		/// <returns>Minimum distance value.</returns>
		inline Float GetMinDistance() const
		{
			return sound->getMinDistance();
		}

		/// <summary>
		/// Sets the maximum distance beyond which sound is no longer attenuated.
		/// </summary>
		/// <param name="_distance">Maximum distance value.</param>
		inline void SetMaxDistance(Float _distance) const
		{
			sound->setMaxDistance(_distance);
		}

		/// <summary>
		/// Gets the maximum distance for attenuation.
		/// </summary>
		/// <returns>Maximum distance value.</returns>
		inline Float GetMaxDistance() const
		{
			return sound->getMaxDistance();
		}

		/// <summary>
		/// Sets the Doppler effect scaling factor.
		/// </summary>
		/// <param name="_factor">Doppler scaling factor.</param>
		inline void SetDopplerFactor(Float _factor) const
		{
			sound->setDopplerFactor(_factor);
		}

		/// <summary>
		/// Gets the Doppler effect scaling factor.
		/// </summary>
		/// <returns>Doppler scaling factor.</returns>
		inline Float GetDopplerFactor() const
		{
			return sound->getDopplerFactor();
		}

		/// <summary>
		/// Sets the directional attenuation factor for sound cones.
		/// </summary>
		/// <param name="_factor">Directional attenuation factor.</param>
		inline void SetDirectionalAttenuationFactor(Float _factor) const
		{
			sound->setDirectionalAttenuationFactor(_factor);
		}

		/// <summary>
		/// Gets the directional attenuation factor.
		/// </summary>
		/// <returns>Directional attenuation factor.</returns>
		inline Float GetDirectionalAttenuationFactor() const
		{
			return sound->getDirectionalAttenuationFactor();
		}

		/// <summary>
		/// Sets the directional emission cone parameters for this sound source.
		/// </summary>
		/// <param name="_cone">Cone specification (inner angle, outer angle, outer gain).</param>
		inline void SetCone(const sf::SoundSource::Cone& _cone)
		{
			return sound->setCone(_cone);
		}

		/// <summary>
		/// Gets the directional emission cone parameters.
		/// </summary>
		/// <returns>Current emission cone.</returns>
		inline sf::SoundSource::Cone GetCone() const
		{
			return sound->getCone();
		}

		/// <summary>
		/// Sets the 3D direction vector that the sound source is pointing towards.
		/// </summary>
		/// <param name="_direction">Direction 3D vector.</param>
		inline void SetDirection(const FVector3& _direction) const
		{
			sound->setDirection(_direction);
		}

		/// <summary>
		/// Gets the 3D direction vector of the sound source.
		/// </summary>
		/// <returns>Direction 3D vector.</returns>
		inline FVector3 GetDirection() const
		{
			return sound->getDirection();
		}

		/// <summary>
		/// Default constructor for Sound.
		/// </summary>
		Sound() = default;

		/// <summary>
		/// Constructs a Sound initialized with a shared sound buffer.
		/// </summary>
		/// <param name="_buffer">Shared pointer to the SFML sound buffer.</param>
		inline Sound(std::shared_ptr<sf::SoundBuffer> _buffer)
		{
			buffer = std::move(_buffer);
			sound = std::make_unique<sf::Sound>(*buffer);
			SetVolume(50.0f);
			SetSpatializationEnabled(false);
		}

		/// <summary>
		/// Gets the shared sound buffer attached to this sound.
		/// </summary>
		/// <returns>Reference to the shared pointer of the sound buffer.</returns>
		inline const std::shared_ptr<sf::SoundBuffer>& GetSharedBuffer() const
		{
			return buffer;
		}

	};

}
