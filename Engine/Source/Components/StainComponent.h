#pragma once
#include "Component.h"
#include "Utilities/Debug/Color.h"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Clock.hpp>
#include <memory>
#include <vector>

namespace Krampus
{
	class Window;
	class Level;

	/// <summary>Look of one mark (saved as an int: never reorder).</summary>
	enum class StainStyle
	{
		Soft,		// a smooth round mark (paint, dirt, light)
		Splat,		// a blob with drops around it (blood, paint)
		Scorch		// an irregular burnt mark (explosions, fire)
	};

	/// <summary>
	/// Lets the actor's sprite be marked: burnt by an explosion, splashed with blood, painted... The marks stay on the sprite (they move,
	/// turn and scale with it) and only show where the sprite is (its shape and the see-through parts of its picture).
	/// Cheap: the marks are painted once into a small picture of the actor, drawn over the sprite in one more draw call;
	/// a thousand marks cost the same as one. Needs a SpriteComponent on the same actor.
	/// Particles mark it (ParticleComponent stainMode), and code with Stain() or StainAt().
	/// </summary>
	class StainComponent : public Component
	{
		struct Stamp
		{
			sf::Vector2f	position;	// in the sprite's own coordinates
			float			radius;
			sf::Color		color;
			int				style;
			float			rotation;	// radians
		};

		std::unique_ptr<sf::RenderTexture>	target;			// the marks, over the sprite's local bounds
		sf::FloatRect						targetBounds;	// the local bounds the target covers
		std::vector<Stamp>					pending;		// painted at the next draw (one draw call for all)
		std::vector<sf::Vertex>				stampVertices;
		bool								hasStains		= false;
		float								fadeCarry		= 0.f;
		float								fadeLeft		= 0.f;		// opacity (0..255) the strongest mark may still have
		sf::Clock							fadeClock;
		std::uint32_t						random			= 0x6D2B79F5u;

	public:
		float			resolution	= 1.f;		// pixels of the marks picture per pixel of the sprite (lower: less memory, softer marks)
		float			fadeSpeed	= 0.f;		// part of the marks that goes away per second (0: they stay forever)
		float			strength	= 1.f;		// multiplies the opacity of every mark it gets (0: it cannot be marked)

		StainComponent(Actor* _owner);

		void Construct() override;
		void Deconstruct() override;
		void BeginPlay() override;

		/// <summary>A mark of _radius pixels (world) at a world position. Painted at the next frame.</summary>
		void Stain(const FVector2& _worldPosition, const float& _radius, const Color& _color, const StainStyle& _style = StainStyle::Soft);

		/// <summary>Removes every mark.</summary>
		void Clear();

		bool HasStains() const { return hasStains || !pending.empty(); }

		/// <returns>True when the world position is on the sprite (its shape).</returns>
		bool Contains(const FVector2& _worldPosition) const;

		/// <summary>
		/// Marks every StainComponent of the level the circle touches (_ignore: an actor left clean, such as the one that made the effect).
		/// </summary>
		/// <returns>How many actors were marked.</returns>
		static int StainAt(Level* _level, const FVector2& _worldPosition, const float& _radius, const Color& _color, const StainStyle& _style = StainStyle::Soft, const Actor* _ignore = nullptr);

		/// <returns>The StainComponent whose sprite is under the world position, or null.</returns>
		static StainComponent* FindAt(Level* _level, const FVector2& _worldPosition, const Actor* _ignore = nullptr);

		/// <summary>The StainComponents of a level and the world rectangle of their sprite, made once per frame by whoever tests many points.</summary>
		struct Candidate
		{
			StainComponent*	stain;
			sf::FloatRect	bounds;
		};
		static void GatherCandidates(Level* _level, const Actor* _ignore, std::vector<Candidate>& _out);

		/// <summary>Called by the SpriteComponent right after it is drawn: paints the waiting marks, then draws them over it.</summary>
		void DrawOver(Window& _window, const sf::Shape& _shape);

		std::string ToString() const override;

	private:
		const sf::Shape* GetSpriteShape() const;
		bool EnsureTarget(const sf::Shape& _shape);
		void PaintPending();
		void Fade(const float& _deltaTime);
		float Random01();
	};
}
