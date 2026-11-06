#pragma once
#include "WidgetComponents/WidgetComponent.h"
#include "GameFramework/SceneObject.h"
#include <optional>

namespace Krampus
{

	/// <summary>
	/// Base class for all 2D screen-space heads-up display (HUD) widgets.
	/// </summary>
	class Widget : public SceneObject<Widget, WidgetComponent>
	{
	public:
		explicit Widget(Level* _level);

		friend class HUD;

		// ---- Layout and clipping (the boxes of ControlWidget.h)

		/// <summary>
		/// The rectangle the widget covers on the screen: its image and label components, or its size for a ControlWidget.
		/// A widget that shows nothing is a point at its position. Boxes place their children with it.
		/// </summary>
		virtual FRect GetBounds() const;

		/// <summary>The children of this widget are only drawn (and clicked) inside this rectangle: a scroll box. None by default.</summary>
		virtual std::optional<FRect> GetChildClip() const { return std::nullopt; }

		/// <summary>The rectangle this widget may draw in, from the clipping widgets above it (none: the whole screen).</summary>
		std::optional<FRect> GetClip() const;

		/// <summary>A view that draws what _base shows, only inside _rect (in the coordinates of _base). None when they do not meet.</summary>
		static std::optional<sf::View> ClipView(const sf::View& _base, const FRect& _rect);

		/// <summary>How many widgets clip their children (the HUD skips the clipping work when there is none).</summary>
		inline static int clippingWidgets = 0;

	protected:
		void OnDestroy() override;
	};

	extern template class SceneObject<Widget, WidgetComponent>;

}
