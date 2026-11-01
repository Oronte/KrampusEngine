#pragma once
#include "Widget.h"
#include "Utilities/System/Drawable.h"
#include "Graphics/Ressources/Font.h"

namespace Krampus
{
	/// <summary>
	/// Base of the widgets that draw themselves (boxes, slider, text entry...): a rectangle of `size` centered on the widget,
	/// scaled like it (not turned: a rotation is ignored), and the mouse / keyboard in the HUD view.
	/// </summary>
	class ControlWidget : public Widget, public IDrawable
	{
		bool									registered		= false;	// in the screen list of the camera manager
		ListenerHandle							mouseMoveHandle;
		ListenerHandle							mousePressHandle;
		ListenerHandle							mouseReleaseHandle;
		ListenerHandle							wheelHandle;
		ListenerHandle							textHandle;

	public:
		/// <summary>Width and height, before the scale of the widget.</summary>
		FVector2								size;
		Color									backgroundColor	= Color(0, 0, 0, 0);
		Color									outlineColor	= Color(0, 0, 0, 0);
		Float									outlineThickness = 0.f;

		ControlWidget(Level* _level, const FVector2& _size);

		/// <summary>The rectangle on the screen (HUD view): size times the scale, centered on the position.</summary>
		FRect GetRect() const;

		FRect GetBounds() const override { return GetRect(); }

		void SetZOrder(const uint8_t& _zOrder) override;
		void SetZOrder(const ZOrder& _zOrder) override;

		/// <summary>Places the children (boxes), then draws the control.</summary>
		void Draw(Window& _window) override;

		/// <summary>Where the mouse is in the HUD view (updated when it moves).</summary>
		FVector2 GetMouse() const { return mouse; }
		bool IsHovered() const { return hovered; }

	protected:
		FVector2								mouse;
		bool									hovered			= false;
		Font									font;

	public:
		/// <summary>Registers the control to be drawn and listens to the mouse and keyboard (the HUD calls it).</summary>
		void Construct() override;
		void Deconstruct() override;

	protected:

		/// <summary>The children are placed here, before the control is drawn (every frame, in the editor too).</summary>
		virtual void Layout() {}
		/// <summary>Draws the control (the background and outline are drawn before).</summary>
		virtual void DrawControl(Window& _window) {}

		// Input, only while the widget is active and visible (not out of a scroll box). The mouse is in the HUD view.
		virtual void OnMouseMove() {}
		virtual void OnMouseDown() {}
		virtual void OnMouseUp() {}
		virtual void OnWheel(const float& _delta) {}
		virtual void OnText(const std::uint32_t& _character) {}

		/// <summary>The mouse is on the rectangle, and inside the scroll boxes the widget is in.</summary>
		bool IsUnderMouse() const;
		bool AcceptsInput() const;

		// Drawing helpers, in the HUD view.
		static void FillRect(Window& _window, const FRect& _rect, const Color& _fill, const Color& _outline = Color(0, 0, 0, 0), const float& _thickness = 0.f);
		static void FillCircle(Window& _window, const FVector2& _center, const float& _radius, const Color& _fill, const Color& _outline = Color(0, 0, 0, 0), const float& _thickness = 0.f);
		/// <summary>A line of text: _alignX 0 left of _anchor, 0.5 centered, 1 right of it; always centered on _anchor.y. Returns its width.</summary>
		float DrawLabel(Window& _window, const sf::String& _text, const FVector2& _anchor, const unsigned& _characterSize, const Color& _color, const float& _alignX = 0.f) const;
		float TextWidth(const sf::String& _text, const unsigned& _characterSize) const;
		/// <summary>Draws with _draw only inside _rect (and inside the rectangle the widget is already clipped to).</summary>
		static void DrawClipped(Window& _window, const FRect& _rect, const std::function<void()>& _draw);
		/// <summary>The scale of the widget as one number (for text and thicknesses).</summary>
		float UniformScale() const;
	};

	/// <summary>UTF-8 (how strings are saved) to the characters SFML draws, and back.</summary>
	sf::String FromUtf8(const std::string& _text);
	std::string ToUtf8(const sf::String& _text);
}
