#pragma once
#include "WidgetComponent.h"
#include "Graphics/Ressources/Text.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/Drawable.h"

namespace Krampus
{
    /// <summary>
    /// Component that renders text strings within a HUD screen widget.
    /// </summary>
    class LabelWidgetComponent : public WidgetComponent, public IDrawable, public IDebuggable
    {
        Text    text;

        Event<FVector2>::ListenerHandle onOwnerMoveHandle;
        Event<Angle>::ListenerHandle onOwnerRotateHandle;
        Event<FVector2>::ListenerHandle onOwnerScaleHandle;

	public:
        /// <summary>Sets the displayed text string.</summary>
        /// <param name="_string">Text string.</param>
        inline void SetString(const String& _string)
        {
            text.SetString(_string);
        }

        /// <summary>Gets the displayed text string.</summary>
        /// <returns>Text string.</returns>
        inline String GetString() const
        {
            return text.GetString();
        }

        /// <summary>Sets the font used to render text.</summary>
        /// <param name="_fileName">Font resource name.</param>
        /// <param name="_type">Font extension type.</param>
        inline void SetFont(const String& _fileName, const FontExtensionType& _type)
        {
            text.SetFont(_fileName, _type);
        }

        /// <summary>Gets the font used by text.</summary>
        /// <returns>Const reference to Font.</returns>
        inline const Font& GetFont() const
        {
            return text.GetFont();
        }

        /// <summary>Sets the character font size in pixels.</summary>
        /// <param name="_size">Character size.</param>
        inline void SetCharacterSize(const UInt& _size)
        {
            text.SetCharacterSize(_size);
        }

        /// <summary>Gets character font size.</summary>
        /// <returns>Character size.</returns>
        inline UInt GetCharacterSize() const
        {
            return text.GetCharacterSize();
        }

        /// <summary>Sets the text fill color.</summary>
        /// <param name="_color">Fill color.</param>
        inline void SetFillColor(const Color& _color)
        {
            text.SetFillColor(_color);
        }

        /// <summary>Gets the text fill color.</summary>
        /// <returns>Fill color.</returns>
        inline Color GetFillColor() const
        {
            return text.GetFillColor();
        }

        /// <summary>Sets the outline border color.</summary>
        /// <param name="_color">Outline color.</param>
        inline void SetOutlineColor(const Color& _color)
        {
            text.SetOutlineColor(_color);
        }

        /// <summary>Gets the outline border color.</summary>
        /// <returns>Outline color.</returns>
        inline Color GetOutlineColor() const
        {
            return text.GetOutlineColor();
        }

        /// <summary>Sets the outline thickness in pixels.</summary>
        /// <param name="_thickness">Outline thickness.</param>
        inline void SetOutlineThickness(const Float& _thickness)
        {
            text.SetOutlineThickness(_thickness);
        }

        /// <summary>Gets the outline thickness in pixels.</summary>
        /// <returns>Outline thickness.</returns>
        inline Float GetOutlineThickness() const
        {
            return text.GetOutlineThickness();
        }

        /// <summary>Sets text style flags (Bold, Italic, Underlined, StrikeThrough).</summary>
        /// <param name="_style">Style bitmask flags.</param>
        inline void SetStyle(const UInt& _style)
        {
            text.SetStyle(_style);
        }

        /// <summary>Gets text style flags.</summary>
        /// <returns>Style flags.</returns>
        inline UInt GetStyle() const
        {
            return text.GetStyle();
        }

        /// <summary>Sets spacing factor between characters.</summary>
        /// <param name="_spacing">Letter spacing.</param>
        inline void SetLetterSpacing(const Float& _spacing)
        {
            text.SetLetterSpacing(_spacing);
        }

        /// <summary>Gets spacing factor between characters.</summary>
        /// <returns>Letter spacing.</returns>
        inline Float GetLetterSpacing() const
        {
            return text.GetLetterSpacing();
        }

        /// <summary>Sets spacing factor between lines.</summary>
        /// <param name="_spacing">Line spacing.</param>
        inline void SetLineSpacing(const Float& _spacing)
        {
            text.SetLineSpacing(_spacing);
        }

        /// <summary>Gets spacing factor between lines.</summary>
        /// <returns>Line spacing.</returns>
        inline Float GetLineSpacing() const
        {
            return text.GetLineSpacing();
        }

        /// <summary>Sets 2D position coordinates.</summary>
        /// <param name="_position">Position vector.</param>
        inline void SetPosition(const FVector2& _position)
        {
            text.SetPosition(_position);
        }

        /// <summary>Gets 2D position coordinates.</summary>
        /// <returns>Position vector.</returns>
        inline FVector2 GetPosition() const
        {
            return text.GetPosition();
        }

        /// <summary>Sets local origin pivot coordinates.</summary>
        /// <param name="_origin">Origin vector.</param>
        inline void SetOrigin(const FVector2& _origin)
        {
            text.SetOrigin(_origin);
        }

        /// <summary>Gets local origin pivot coordinates.</summary>
        /// <returns>Origin vector.</returns>
        inline FVector2 GetOrigin() const
        {
            return text.GetOrigin();
        }

        /// <summary>Sets 2D scale factors.</summary>
        /// <param name="_factors">Scale multipliers.</param>
        inline void SetScale(const FVector2& _factors)
        {
            text.SetScale(_factors);
        }

        /// <summary>Gets 2D scale factors.</summary>
        /// <returns>Scale multipliers.</returns>
        inline FVector2 GetScale() const
        {
            return text.GetScale();
        }

        /// <summary>Sets rotation angle.</summary>
        /// <param name="_angle">Angle.</param>
        inline void SetRotation(const Angle& _angle)
        {
            text.SetRotation(_angle);
        }

        /// <summary>Gets rotation angle.</summary>
        /// <returns>Current Angle.</returns>
        inline Angle GetRotation() const
        {
            return Angle(text.GetRotation());
        }

        /// <summary>Sets combined transform (position, rotation, scale).</summary>
        /// <param name="_transform">Transform parameters.</param>
        inline void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position);
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }

        /// <summary>Gets combined transform.</summary>
        /// <returns>Transform structure.</returns>
        inline Transform GetTransform() const
        {
            return Transform(GetPosition(), GetRotation(), GetScale());
        }

        /// <summary>Gets local untransformed bounding box.</summary>
        /// <returns>Local bounds FRect.</returns>
        inline FRect GetLocalBounds() const
        {
            return text.GetLocalBounds();
        }

        /// <summary>Gets transformed global bounding box.</summary>
        /// <returns>Global bounds FRect.</returns>
        inline FRect GetGlobalBounds() const
        {
            return text.GetGlobalBounds();
        }

        /// <summary>Finds position of glyph at index.</summary>
        /// <param name="_index">Glyph index.</param>
        /// <returns>Position vector.</returns>
        inline FVector2 FindCharacterPos(const Int& _index) const
        {
            return text.FindCharacterPos(_index);
        }

        /// <summary>Constructs a LabelWidgetComponent with initial text configuration.</summary>
        /// <param name="_owner">Owning Widget.</param>
        /// <param name="_data">Text data settings.</param>
		LabelWidgetComponent(Widget* _owner, const TextData& _data);

	private:
		void Draw(Window& _window) override;
        void DrawDebug() override;

    public:
        /// <summary>Lifecycle callback on component creation.</summary>
        virtual void Construct() override;
        /// <summary>Lifecycle callback on component deconstruction.</summary>
        virtual void Deconstruct() override;

        /// <summary>Sets numeric Z-order render layer.</summary>
        /// <param name="_zOrder">Numeric Z-order.</param>
        virtual void SetZOrder(const uint8_t& _zOrder) override;

        /// <summary>Sets Z-order render layer using ZOrder enum.</summary>
        /// <param name="_zOrder">ZOrder enum value.</param>
        virtual void SetZOrder(const ZOrder& _zOrder) override;

    public:
        /// <summary>Returns a string representation of the component.</summary>
        /// <returns>String representation.</returns>
        virtual std::string ToString() const override;
    };

}
