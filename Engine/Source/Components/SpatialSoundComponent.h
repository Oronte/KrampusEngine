#pragma once
#include "Component.h"
#include "Managers/AudioManager.h"

namespace Krampus
{

	/// <summary>
	/// Sound emitted from the owner's position: follows its moves and rotation, and its speed for the Doppler effect.
	/// Volume, pitch, loop and 3D settings are set on GetSound().
	/// </summary>
	class SpatialSoundComponent : public Component
	{
		Sound sound;

		FVector2 previousPosition;
		Event<FVector2>::ListenerHandle onOwnerMoveHandle;
		Event<Angle>::ListenerHandle onOwnerRotateHandle;

	public:
		/// <summary>Whether spatialization uses a top-down view (forward = up vector) or a side view (forward = right vector).</summary>
		Bool isTopDown			= false;

		/// <summary>Whether the sound starts playing on BeginPlay.</summary>
		Bool playOnBeginPlay	= false;

		/// <summary>Sound file shown in the Inspector (name without extension, empty = silent) and its format.</summary>
		String soundFile;
		AudioExtensionType soundType = AudioExtensionType::MP3;

		explicit SpatialSoundComponent(Actor* _owner);
		SpatialSoundComponent(Actor* _owner, const String& _fileName, const AudioExtensionType& _type);

		/// <summary>
		/// Replaces the sound by a file of Content/Audio, keeping volume, loop, pitch and attenuation. An empty name or a missing file makes it silent.
		/// </summary>
		void SetSoundFile(const String& _fileName, const AudioExtensionType& _type);

		[[nodiscard]] Sound& GetSound() { return sound; }
		[[nodiscard]] const Sound& GetSound() const { return sound; }

		void Play();
		void Pause() { sound.Pause(); }
		void Stop();

		void Construct() override;
		void BeginPlay() override;
		void Tick(const Float& _deltaTime) override;

	private:
		void ApplySpatialSettings();
		void FollowOwner();
		void UpdateDirection();
	};

}
