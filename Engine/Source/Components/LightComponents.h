#pragma once
#include "Component.h"
#include <vector>

// 2D lights. As soon as a level has a light (PointLightComponent or GlobalLightComponent), the world is drawn lit: what no
// light reaches has the global light's color (dark by default), each point light adds its color around it, and a
// ShadowCasterComponent stops the light of the point lights that cast shadows. The HUD is never lit.
//
//     Light (actor, Hierarchy > Add Actor)    a PointLightComponent
//     Global Light                            a GlobalLightComponent: the ambient color of the level
//     a ShadowCasterComponent on a sprite     its outline makes a shadow

namespace Krampus
{
	class Level;

	/// <summary>
	/// A round light: full color at the center, nothing at radius.
	/// </summary>
	class PointLightComponent : public Component
	{
	public:
		Color	color			= Color(255, 225, 170);
		float	intensity		= 1.f;		// 2 = twice as bright (the color saturates)
		float	radius			= 300.f;	// pixels
		bool	castShadows		= true;		// the ShadowCasterComponents stop it

		PointLightComponent(Actor* _owner);

		void Construct() override;
		void Deconstruct() override;

		static std::vector<PointLightComponent*> LightsOf(const Level* _level);
	};

	/// <summary>
	/// The light everywhere: the color of what no point light reaches. Without one, a level with point lights is dark around them.
	/// </summary>
	class GlobalLightComponent : public Component
	{
	public:
		Color	color			= Color(40, 44, 60);
		float	intensity		= 1.f;

		GlobalLightComponent(Actor* _owner);

		void Construct() override;
		void Deconstruct() override;

		/// <summary>The global light of the level (the first one), or nullptr.</summary>
		static GlobalLightComponent* Of(const Level* _level);
	};

	/// <summary>
	/// Makes the outline of the actor's sprite stop the point lights: a shadow behind it, the sprite itself stays lit.
	/// </summary>
	class ShadowCasterComponent : public Component
	{
	public:
		ShadowCasterComponent(Actor* _owner);

		void Construct() override;
		void Deconstruct() override;

		/// <summary>The outline in the world (the sprite's, or nothing without a sprite).</summary>
		std::vector<FVector2> GetWorldOutline() const;

		static std::vector<ShadowCasterComponent*> CastersOf(const Level* _level);
	};
}
