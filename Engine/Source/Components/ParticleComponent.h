#pragma once
#include "Component.h"
#include "Graphics/Particles/ParticleEmitter.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/Drawable.h"
#include "Utilities/System/Event.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Clock.hpp>
#include <memory>

namespace Krampus
{

	/// <summary>
	/// A particle effect on an actor (fire, smoke, sparks, explosion, snow, rain, magic...): pick a preset in the Inspector, then change
	/// what you want. Drawn in one call for all its particles, with nothing drawn when it is out of the view.
	/// In the editor it plays in the Viewport too (previewInEditor), without the level running.
	/// </summary>
	class ParticleComponent : public Component, public IDrawable, public IDebuggable
	{
		ParticleEmitter							emitter;
		int										preset				= static_cast<int>(ParticlePreset::Fire);
		std::vector<sf::Vertex>					vertices;			// written each frame, keeps its size
		std::shared_ptr<sf::Texture>			texture;			// null: the soft round dot
		std::string								loadedTexture;		// settings.texture when the texture was loaded
		sf::Clock								previewClock;		// time in the editor, where the level does not tick
		bool									previewStarted		= false;
		float									previewWait			= 0.f;		// seconds since a one-shot effect ended, in the editor

	public:
		bool									previewInEditor		= true;		// plays in the Viewport while editing
		bool									destroyOwnerWhenDone = false;	// a one-shot effect (explosion) removes its actor when it ends

		/// <summary>Said once when the effect has ended (not looping, last particle gone).</summary>
		Event<>									onFinished;

		ParticleComponent(Actor* _owner, const ParticlePreset& _preset = ParticlePreset::Fire);

		void Construct() override;
		void Deconstruct() override;
		void BeginPlay() override;
		void Tick(const Float& _deltaTime) override;
		void Draw(Window& _window) override;

		void SetZOrder(const uint8_t& _zOrder) override;
		void SetZOrder(const ZOrder& _zOrder) override;

		/// <summary>The values of the effect: change them at any time, they apply at once.</summary>
		ParticleSettings& Settings() { return emitter.settings; }
		const ParticleSettings& Settings() const { return emitter.settings; }

		/// <summary>Replaces the settings by the ones of a ready-made effect (the texture is kept).</summary>
		void ApplyPreset(const ParticlePreset& _preset);
		ParticlePreset GetPreset() const { return static_cast<ParticlePreset>(preset); }

		/// <summary>Starts a cycle again (with its burst).</summary>
		void Play() { emitter.Play(); }

		/// <summary>No new particle; the ones there end their life.</summary>
		void Stop() { emitter.Stop(); }

		/// <summary>Removes every particle now.</summary>
		void Clear() { emitter.Clear(); }

		/// <summary>Removes everything and starts from nothing.</summary>
		void Restart() { emitter.Clear(); emitter.Play(); }

		/// <summary>_count particles at once.</summary>
		void Burst(const int& _count) { emitter.Burst(_count); }

		bool IsPlaying() const { return emitter.IsPlaying(); }
		int GetAliveCount() const { return emitter.GetAliveCount(); }

		std::string ToString() const override;

	private:
		void Simulate(const float& _deltaTime);
		void DrawDebug() override;	// the shape particles are born in
	};
}
