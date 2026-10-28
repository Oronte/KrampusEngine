#pragma once
#include "Component.h"
#include "Managers/AudioManager.h"

namespace Krampus
{

	/// <summary>
	/// Component that provides localized 2D/3D spatial sound emission anchored to an actor.
	/// </summary>
	class SpatialSoundComponent : public Component
	{
		Sound sound;

		FVector2 previousPosition;
		Event<FVector2>::ListenerHandle onOwnerMoveHandle;
		Event<Angle>::ListenerHandle onOwnerRotateHandle;

	public:
		/// <summary>
		/// Whether spatialization uses top-down view (Z is up) or side-scroll view (Y is up).
		/// </summary>
		Bool isTopDown			= false;

		/// <summary>
		/// Whether the sound begins playing automatically on BeginPlay.
		/// </summary>
		Bool playOnBeginPlay	= false;

		/// <summary>Sound file shown in the Inspector (name without extension, empty = silent) and its format.</summary>
		String soundFile;
		AudioExtensionType soundType = AudioExtensionType::MP3;

		/// <summary>
		/// Replaces the sound by a file of Content/Audio, keeping volume, loop, pitch and attenuation. An empty name or a missing file makes it silent.
		/// </summary>
		void SetSoundFile(const String& _fileName, const AudioExtensionType& _type);

	public:
		/// <summary>
		/// Constructs a SpatialSoundComponent attached to the specified actor.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		SpatialSoundComponent(Actor* _owner);

		/// <summary>
		/// Constructs a SpatialSoundComponent attached to the specified actor and loads the specified audio file.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_fileName">Audio file name.</param>
		/// <param name="_type">Audio file extension type.</param>
		SpatialSoundComponent(Actor* _owner, const String& _fileName, const AudioExtensionType& _type);

		/// <summary>
		/// Lifecycle callback invoked when the component is initialized.
		/// </summary>
		virtual void Construct() override;

		/// <summary>
		/// Lifecycle callback invoked when gameplay begins, optionally auto-playing if configured.
		/// </summary>
		virtual void BeginPlay() override;

		/// <summary>
		/// Updates the sound position, velocity, and orientation based on owner movements.
		/// </summary>
		/// <param name="_deltaTime">Elapsed frame time in seconds.</param>
		virtual void Tick(const Float& _deltaTime) override;

	private:
		void ApplySpatialSettings();

	public:

		/// <summary>
		/// Gets the volume of the sound in the range [0, 100].
		/// </summary>
		/// <returns>Current volume.</returns>
		inline virtual Float GetVolume() const 
		{
			return sound.GetVolume();
		}

		/// <summary>
		/// Sets the volume of the sound in the range [0, 100].
		/// </summary>
		/// <param name="_volume">New volume level.</param>
		inline virtual void SetVolume(const Float& _volume) 
		{
			sound.SetVolume(_volume);
		}

		/// <summary>
		/// Starts or resumes audio playback.
		/// </summary>
		inline virtual void Play() 
		{
			LOG(VerbosityType::Log, "Sound: play '" + soundFile.StdString() + "' on " + GetName().StdString());
			sound.Play();
		}

		/// <summary>
		/// Pauses audio playback.
		/// </summary>
		inline virtual void Pause() 
		{
			sound.Pause();
		}

		/// <summary>
		/// Stops audio playback and rewinds to the beginning.
		/// </summary>
		inline virtual void Stop() 
		{
			LOG(VerbosityType::Log, "Sound: stop '" + soundFile.StdString() + "' on " + GetName().StdString());
			sound.Stop();
		}

		/// <summary>
		/// Checks whether the sound loops continuously.
		/// </summary>
		/// <returns>True if loop is enabled, false otherwise.</returns>
		inline virtual Bool IsLooping() const 
		{
			return sound.IsLooping();
		}

		/// <summary>
		/// Sets whether the sound loops continuously.
		/// </summary>
		/// <param name="_loop">True to enable looping, false to disable.</param>
		inline virtual void SetLoop(const Bool& _loop) 
		{
			sound.SetLoop(_loop);
		}

		/// <summary>
		/// Gets the pitch multiplier of the sound.
		/// </summary>
		/// <returns>Pitch multiplier.</returns>
		inline virtual Float GetPitch() const 
		{
			return sound.GetPitch();
		}

		/// <summary>
		/// Sets the pitch multiplier of the sound.
		/// </summary>
		/// <param name="_pitch">New pitch multiplier.</param>
		inline virtual void SetPitch(const Float& _pitch) 
		{
			sound.SetPitch(_pitch);
		}

		/// <summary>
		/// Adds an offset to the current pitch multiplier.
		/// </summary>
		/// <param name="_pitch">Pitch delta to add.</param>
		inline virtual void AddPitch(const Float& _pitch) 
		{
			SetPitch(GetPitch() + _pitch);
		}

		/// <summary>
		/// Checks whether the sound is available to play.
		/// </summary>
		/// <returns>True if stopped or paused, false if playing.</returns>
		inline virtual Bool IsAvaliable() const 
		{
			return !IsPlaying();
		}

		/// <summary>
		/// Checks whether the sound is currently playing.
		/// </summary>
		/// <returns>True if playing, false otherwise.</returns>
		inline virtual Bool IsPlaying() const 
		{
			return sound.IsPlaying();
		}

		/// <summary>
		/// Checks whether the sound is currently paused.
		/// </summary>
		/// <returns>True if paused, false otherwise.</returns>
		inline virtual Bool IsPaused() const 
		{
			return sound.IsPaused();
		}

		/// <summary>
		/// Checks whether the sound is currently stopped.
		/// </summary>
		/// <returns>True if stopped, false otherwise.</returns>
		inline virtual Bool IsStopped() const 
		{
			return sound.IsStopped();
		}

		/// <summary>
		/// Sets the distance attenuation factor.
		/// </summary>
		/// <param name="_attenuation">Attenuation factor.</param>
		inline virtual void SetAttenuation(Float _attenuation) 
		{
			sound.SetAttenuation(_attenuation);
		}

		/// <summary>
		/// Gets the distance attenuation factor.
		/// </summary>
		/// <returns>Attenuation factor.</returns>
		inline virtual Float GetAttenuation() const 
		{
			return sound.GetAttenuation();
		}

		/// <summary>
		/// Enables or disables 3D spatialization.
		/// </summary>
		/// <param name="_enable">True to enable spatialization, false to disable.</param>
		inline void SetSpatializationEnabled(Bool _enable) const 
		{
			sound.SetSpatializationEnabled(_enable);
		}

		/// <summary>
		/// Checks whether 3D spatialization is enabled.
		/// </summary>
		/// <returns>True if enabled, false otherwise.</returns>
		inline Bool IsSpatializationEnabled() const 
		{
			return sound.IsSpatializationEnabled();
		}

		/// <summary>
		/// Sets whether position is relative to the listener.
		/// </summary>
		/// <param name="_relative">True for relative positioning, false for absolute.</param>
		inline void SetRelativeToListener(Bool _relative) const 
		{
			sound.SetRelativeToListener(_relative);
		}

		/// <summary>
		/// Checks whether position is relative to the listener.
		/// </summary>
		/// <returns>True if relative, false otherwise.</returns>
		inline Bool IsRelativeToListener() const 
		{
			return sound.IsRelativeToListener();
		}

		/// <summary>
		/// Sets the 3D velocity vector of the sound source.
		/// </summary>
		/// <param name="_velocity">Velocity 3D vector.</param>
		inline void SetVelocity(const FVector3& _velocity) const 
		{
			sound.SetVelocity(_velocity);
		}

		/// <summary>
		/// Gets the 3D velocity vector of the sound source.
		/// </summary>
		/// <returns>Velocity 3D vector.</returns>
		inline FVector3 GetVelocity() const 
		{
			return sound.GetVelocity();
		}

		/// <summary>
		/// Sets the 3D world position of the sound source directly.
		/// </summary>
		/// <param name="_position">3D position coordinates.</param>
		inline void Set3DPosition(const FVector3& _position) const 
		{
			sound.SetPosition(_position);
		}

		/// <summary>
		/// Gets the 3D world position of the sound source.
		/// </summary>
		/// <returns>3D position coordinates.</returns>
		inline FVector3 Get3DPosition() const 
		{
			return sound.GetPosition();
		}

		/// <summary>
		/// Sets the current playback offset in seconds.
		/// </summary>
		/// <param name="_time">Offset in seconds.</param>
		inline void SetPlayingOffset(Float _time) const 
		{
			sound.SetPlayingOffset(_time);
		}

		/// <summary>
		/// Gets the current playback offset in seconds.
		/// </summary>
		/// <returns>Offset in seconds.</returns>
		inline Float GetPlayingOffset() const 
		{
			return sound.GetPlayingOffset();
		}

		/// <summary>
		/// Sets the stereo panning value in [-1.0, 1.0].
		/// </summary>
		/// <param name="_pan">Pan value.</param>
		inline void SetPan(Float _pan) const 
		{
			sound.SetPan(_pan);
		}

		/// <summary>
		/// Gets the stereo panning value.
		/// </summary>
		/// <returns>Pan value.</returns>
		inline Float GetPan() const 
		{
			return sound.GetPan();
		}

		/// <summary>
		/// Sets the minimum gain floor.
		/// </summary>
		/// <param name="_gain">Minimum gain.</param>
		inline void SetMinGain(Float _gain) const 
		{
			sound.SetMinGain(_gain);
		}

		/// <summary>
		/// Gets the minimum gain floor.
		/// </summary>
		/// <returns>Minimum gain.</returns>
		inline Float GetMinGain() const 
		{
			return sound.GetMinGain();
		}

		/// <summary>
		/// Sets the maximum gain ceiling.
		/// </summary>
		/// <param name="_gain">Maximum gain.</param>
		inline void SetMaxGain(Float _gain) const 
		{
			sound.SetMaxGain(_gain);
		}

		/// <summary>
		/// Gets the maximum gain ceiling.
		/// </summary>
		/// <returns>Maximum gain.</returns>
		inline Float GetMaxGain() const 
		{
			return sound.GetMaxGain();
		}

		/// <summary>
		/// Sets the minimum distance where distance attenuation begins.
		/// </summary>
		/// <param name="_distance">Minimum distance.</param>
		inline void SetMinDistance(Float _distance) const 
		{
			sound.SetMinDistance(_distance);
		}

		/// <summary>
		/// Gets the minimum distance for attenuation.
		/// </summary>
		/// <returns>Minimum distance.</returns>
		inline Float GetMinDistance() const 
		{
			return sound.GetMinDistance();
		}

		/// <summary>
		/// Sets the maximum distance beyond which sound is no longer attenuated.
		/// </summary>
		/// <param name="_distance">Maximum distance.</param>
		inline void SetMaxDistance(Float _distance) const 
		{
			sound.SetMaxDistance(_distance);
		}

		/// <summary>
		/// Gets the maximum distance for attenuation.
		/// </summary>
		/// <returns>Maximum distance.</returns>
		inline Float GetMaxDistance() const 
		{
			return sound.GetMaxDistance();
		}

		/// <summary>
		/// Sets the Doppler effect scaling factor.
		/// </summary>
		/// <param name="_factor">Doppler factor.</param>
		inline void SetDopplerFactor(Float _factor) const 
		{
			sound.SetDopplerFactor(_factor);
		}

		/// <summary>
		/// Gets the Doppler effect scaling factor.
		/// </summary>
		/// <returns>Doppler factor.</returns>
		inline Float GetDopplerFactor() const 
		{
			return sound.GetDopplerFactor();
		}

		/// <summary>
		/// Sets the directional attenuation factor.
		/// </summary>
		/// <param name="_factor">Directional attenuation factor.</param>
		inline void SetDirectionalAttenuationFactor(Float _factor) const 
		{
			sound.SetDirectionalAttenuationFactor(_factor);
		}

		/// <summary>
		/// Gets the directional attenuation factor.
		/// </summary>
		/// <returns>Directional attenuation factor.</returns>
		inline Float GetDirectionalAttenuationFactor() const 
		{
			return sound.GetDirectionalAttenuationFactor();
		}

		/// <summary>
		/// Sets the directional emission cone parameters.
		/// </summary>
		/// <param name="_cone">Emission cone specification.</param>
		inline void SetCone(const sf::SoundSource::Cone& _cone) 
		{
			return sound.SetCone(_cone);
		}

		/// <summary>
		/// Gets the directional emission cone parameters.
		/// </summary>
		/// <returns>Current emission cone.</returns>
		inline sf::SoundSource::Cone GetCone() const 
		{
			return sound.GetCone();
		}

		/// <summary>
		/// Sets the 3D direction vector of the sound emission.
		/// </summary>
		/// <param name="_direction">Direction vector.</param>
		inline void SetDirection(const FVector3& _direction) const 
		{
			sound.SetDirection(_direction);
		}

		/// <summary>
		/// Gets the 3D direction vector of the sound emission.
		/// </summary>
		/// <returns>Direction vector.</returns>
		inline FVector3 GetDirection() const 
		{
			return sound.GetDirection();
		}
	};

}
