#pragma once
#include "Utilities/Macro.h"
#include "Utilities/Math/Vector3D.h"

namespace Krampus
{

	/// <summary>
	/// Playback and 3D controls shared by Sound and Music: thin forwards to the owned SFML source
	/// (sf::Sound or sf::Music). A default-constructed sample owns no source: check IsValid() before using it.
	/// </summary>
	template <typename Source>
	class Sample
	{
	public:
		static constexpr float DEFAULT_VOLUME = 50.f;

		[[nodiscard]] bool IsValid() const { return source != nullptr; }

		void Play()  { source->play(); }
		void Pause() { source->pause(); }
		void Stop()  { source->stop(); }

		[[nodiscard]] bool IsPlaying() const { return source->getStatus() == sf::SoundSource::Status::Playing; }
		[[nodiscard]] bool IsPaused()  const { return source->getStatus() == sf::SoundSource::Status::Paused; }
		[[nodiscard]] bool IsStopped() const { return source->getStatus() == sf::SoundSource::Status::Stopped; }

		/// <summary>Volume in [0, 100].</summary>
		[[nodiscard]] float GetVolume() const { return source->getVolume(); }
		void SetVolume(float _volume)         { source->setVolume(_volume); }

		[[nodiscard]] bool IsLooping() const { return source->isLooping(); }
		void SetLoop(bool _loop)             { source->setLooping(_loop); }

		/// <summary>Pitch multiplier (1 = original speed).</summary>
		[[nodiscard]] float GetPitch() const { return source->getPitch(); }
		void SetPitch(float _pitch)          { source->setPitch(_pitch); }
		void AddPitch(float _delta)          { SetPitch(GetPitch() + _delta); }

		/// <summary>Playing position, in seconds.</summary>
		[[nodiscard]] float GetPlayingOffset() const { return source->getPlayingOffset().asSeconds(); }
		void SetPlayingOffset(float _seconds)        { source->setPlayingOffset(sf::seconds(_seconds)); }

		[[nodiscard]] float GetPan() const { return source->getPan(); }
		void SetPan(float _pan)            { source->setPan(_pan); }

		[[nodiscard]] float GetAttenuation() const { return source->getAttenuation(); }
		void SetAttenuation(float _attenuation)    { source->setAttenuation(_attenuation); }

		[[nodiscard]] bool IsSpatializationEnabled() const { return source->isSpatializationEnabled(); }
		void SetSpatializationEnabled(bool _enabled)       { source->setSpatializationEnabled(_enabled); }

		[[nodiscard]] bool IsRelativeToListener() const { return source->isRelativeToListener(); }
		void SetRelativeToListener(bool _relative)      { source->setRelativeToListener(_relative); }

		[[nodiscard]] FVector3 GetPosition() const  { return source->getPosition(); }
		void SetPosition(const FVector3& _position) { source->setPosition(_position); }

		[[nodiscard]] FVector3 GetDirection() const   { return source->getDirection(); }
		void SetDirection(const FVector3& _direction) { source->setDirection(_direction); }

		[[nodiscard]] FVector3 GetVelocity() const  { return source->getVelocity(); }
		void SetVelocity(const FVector3& _velocity) { source->setVelocity(_velocity); }

		[[nodiscard]] sf::SoundSource::Cone GetCone() const { return source->getCone(); }
		void SetCone(const sf::SoundSource::Cone& _cone)    { source->setCone(_cone); }

		[[nodiscard]] float GetMinGain() const { return source->getMinGain(); }
		void SetMinGain(float _gain)           { source->setMinGain(_gain); }
		[[nodiscard]] float GetMaxGain() const { return source->getMaxGain(); }
		void SetMaxGain(float _gain)           { source->setMaxGain(_gain); }

		[[nodiscard]] float GetMinDistance() const { return source->getMinDistance(); }
		void SetMinDistance(float _distance)       { source->setMinDistance(_distance); }
		[[nodiscard]] float GetMaxDistance() const { return source->getMaxDistance(); }
		void SetMaxDistance(float _distance)       { source->setMaxDistance(_distance); }

		[[nodiscard]] float GetDopplerFactor() const { return source->getDopplerFactor(); }
		void SetDopplerFactor(float _factor)         { source->setDopplerFactor(_factor); }

		[[nodiscard]] float GetDirectionalAttenuationFactor() const { return source->getDirectionalAttenuationFactor(); }
		void SetDirectionalAttenuationFactor(float _factor)         { source->setDirectionalAttenuationFactor(_factor); }

	protected:
		std::unique_ptr<Source> source;

		Sample() = default;
		explicit Sample(std::unique_ptr<Source> _source) : source(std::move(_source))
		{
			SetVolume(DEFAULT_VOLUME);
			SetSpatializationEnabled(false);
		}
	};

}
