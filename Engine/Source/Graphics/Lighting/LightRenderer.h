#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

namespace Krampus
{
	class Level;

	/// <summary>
	/// Draws the light of a level over its world (CameraManager, after the actors, before the post process):
	///   - a light map the size of the window is filled with the global light's color;
	///   - each point light adds a round gradient to it (a light that casts shadows is drawn alone first, the shadows of the
	///     ShadowCasterComponents cut out of it in black, then added);
	///   - the picture is multiplied by the light map: white keeps the colors, black hides them.
	/// Nothing is done in a level without lights.
	/// </summary>
	class LightRenderer
	{
		sf::RenderTexture			lightMap;
		sf::RenderTexture			oneLight;		// a light and its shadows, before it is added
		std::vector<sf::Vertex>		gradient;		// reused
		std::vector<sf::Vertex>		shadows;

	public:
		/// <summary>True when the level has a light: its world is drawn lit.</summary>
		static bool IsLit(const Level* _level);

		/// <summary>Lights what was drawn in the current view of _target (only inside its viewport).</summary>
		void Render(const Level* _level, sf::RenderTarget& _target);
	};
}
