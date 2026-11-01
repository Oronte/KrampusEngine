#pragma once
#include "Component.h"
#include "Graphics/Shaders/ShaderMaterial.h"
#include <vector>

namespace Krampus
{
	class Level;

	/// <summary>
	/// A full-screen effect on the world (Unreal's Post Process Volume, unbound): its shader is drawn over the picture of the level,
	/// after the actors and before the HUD and debug drawings. Several of them stack, the lowest priority first.
	/// Content/Shaders/Builtin/Post has vignette, CRT, chromatic aberration, color grading and blur; any sprite shader works too.
	/// </summary>
	class PostProcessComponent : public Component, public IShaded
	{
	public:
		int		priority		= 0;		// order of the effects: lower first
		bool	applyInEditor	= true;		// shown while editing the scene too

		PostProcessComponent(Actor* _owner);

		void Construct() override;
		void Deconstruct() override;

		/// <returns>The effects to draw for a level, in order.</returns>
		static std::vector<PostProcessComponent*> EffectsOf(const Level* _level, const bool& _editing);

		std::string ToString() const override { return name + " -> " + (material.shader.empty() ? std::string("(no shader)") : material.shader); }
	};
}
