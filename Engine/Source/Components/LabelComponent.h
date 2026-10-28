#pragma once
#include "Components/Component.h"
#include "Graphics/Ressources/Text.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/Drawable.h"

namespace Krampus
{
	/// <summary>
	/// Component that renders text strings attached to an actor.
	/// </summary>
	class LabelComponent : public Component, public IDrawable, public IDebuggable
	{
		Text    text;

        Event<FVector2>::ListenerHandle onOwnerMoveHandle;
        Event<Angle>::ListenerHandle onOwnerRotateHandle;
        Event<FVector2>::ListenerHandle onOwnerScaleHandle;

	public:
		/// <summary>
		/// Sets the displayed text string.
		/// </summary>
		/// <param name="_string">The string content to display.</param>
        inline void SetString(const String& _string)
        {
            text.SetString(_string);
        }

		/// <summary>
		/// Gets the displayed text string.
		/// </summary>
		/// <returns>String content.</returns>
        inline String GetString() const
        {
            return text.GetString();
        }

		/// <summary>
		/// Sets the font used to render the text.
		/// </summary>
		/// <param name="_fileName">Font file name.</param>
		/// <param name="_type">Font file extension type.</param>
        inline void SetFont(const String& _fileName, const FontExtensionType& _type)
        {
            text.SetFont(_fileName, _type);
        }

		/// <summary>
		/// Gets the font used by the text.
		/// </summary>
		/// <returns>Reference to Font.</returns>
        inline const String& GetFontFileName() const
        {
            return text.GetFontFileName();
        }

        inline FontExtensionType GetFontType() const
        {
            return text.GetFontType();
        }

        inline const Font& GetFont() const
        {
            return text.GetFont();
        }

		/// <summary>
		/// Sets the character size in pixels.
		/// </summary>
		/// <param name="_size">Character size.</param>
        inline void SetCharacterSize(const UInt& _size)
        {
            text.SetCharacterSize(_size);
        }

		/// <summary>
		/// Gets the character size in pixels.
		/// </summary>
		/// <returns>Character size.</returns>
        inline UInt GetCharacterSize() const
        {
            return text.GetCharacterSize();
        }

		/// <summary>
		/// Sets the text fill color.
		/// </summary>
		/// <param name="_color">Fill color.</param>
        inline void SetFillColor(const Color& _color)
        {
            text.SetFillColor(_color);
        }

		/// <summary>
		/// Gets the text fill color.
		/// </summary>
		/// <returns>Fill color.</returns>
        inline Color GetFillColor() const
        {
            return text.GetFillColor();
        }

		/// <summary>
		/// Sets the outline color of the text characters.
		/// </summary>
		/// <param name="_color">Outline color.</param>
        inline void SetOutlineColor(const Color& _color)
        {
            text.SetOutlineColor(_color);
        }

		/// <summary>
		/// Gets the outline color of the text characters.
		/// </summary>
		/// <returns>Outline color.</returns>
        inline Color GetOutlineColor() const
        {
            return text.GetOutlineColor();
        }

		/// <summary>
		/// Sets the outline thickness in pixels.
		/// </summary>
		/// <param name="_thickness">Outline thickness.</param>
        inline void SetOutlineThickness(const Float& _thickness)
        {
            text.SetOutlineThickness(_thickness);
        }

		/// <summary>
		/// Gets the outline thickness.
		/// </summary>
		/// <returns>Outline thickness.</returns>
        inline Float GetOutlineThickness() const
        {
            return text.GetOutlineThickness();
        }

		/// <summary>
		/// Sets text styling flags (Regular, Bold, Italic, Underlined, StrikeThrough).
		/// </summary>
		/// <param name="_style">Style bitmask flags.</param>
        inline void SetStyle(const UInt& _style)
        {
            text.SetStyle(_style);
        }

		/// <summary>
		/// Gets the text styling bitmask flags.
		/// </summary>
		/// <returns>Style flags.</returns>
        inline UInt GetStyle() const
        {
            return text.GetStyle();
        }

		/// <summary>
		/// Sets the spacing factor between characters.
		/// </summary>
		/// <param name="_spacing">Letter spacing factor.</param>
        inline void SetLetterSpacing(const Float& _spacing)
        {
            text.SetLetterSpacing(_spacing);
        }

		/// <summary>
		/// Gets the spacing factor between characters.
		/// </summary>
		/// <returns>Letter spacing factor.</returns>
        inline Float GetLetterSpacing() const
        {
            return text.GetLetterSpacing();
        }

		/// <summary>
		/// Sets the spacing factor between lines of text.
		/// </summary>
		/// <param name="_spacing">Line spacing factor.</param>
        inline void SetLineSpacing(const Float& _spacing)
        {
            text.SetLineSpacing(_spacing);
        }

		/// <summary>
		/// Gets the line spacing factor.
		/// </summary>
		/// <returns>Line spacing factor.</returns>
        inline Float GetLineSpacing() const
        {
            return text.GetLineSpacing();
        }

		/// <summary>
		/// Sets the position of the label in 2D space.
		/// </summary>
		/// <param name="_position">New position coordinates.</param>
        inline void SetPosition(const FVector2& _position)
        {
            text.SetPosition(_position);
        }

		/// <summary>
		/// Gets the position of the label in 2D space.
		/// </summary>
		/// <returns>Position coordinates.</returns>
        inline FVector2 GetPosition() const
        {
            return text.GetPosition();
        }

		/// <summary>
		/// Sets the local origin point of the label (pivot point for transformations).
		/// </summary>
		/// <param name="_origin">Origin coordinates.</param>
        inline void SetOrigin(const FVector2& _origin)
        {
            text.SetOrigin(_origin);
        }

		/// <summary>
		/// Gets the local origin point of the label.
		/// </summary>
		/// <returns>Origin coordinates.</returns>
        inline FVector2 GetOrigin() const
        {
            return text.GetOrigin();
        }

		/// <summary>
		/// Sets the 2D scale factor of the label.
		/// </summary>
		/// <param name="_factors">Scale multipliers.</param>
        inline void SetScale(const FVector2& _factors)
        {
            text.SetScale(_factors);
        }

		/// <summary>
		/// Gets the 2D scale factor of the label.
		/// </summary>
		/// <returns>Scale multipliers.</returns>
        inline FVector2 GetScale() const
        {
            return text.GetScale();
        }

		/// <summary>
		/// Sets the rotation angle of the label.
		/// </summary>
		/// <param name="_angle">Rotation angle.</param>
        inline void SetRotation(const Angle& _angle)
        {
            text.SetRotation(_angle);
        }

		/// <summary>
		/// Gets the rotation angle of the label.
		/// </summary>
		/// <returns>Rotation angle.</returns>
        inline Angle GetRotation() const
        {
            return Angle(text.GetRotation());
        }

		/// <summary>
		/// Sets the combined transform (position, rotation, scale) of the label.
		/// </summary>
		/// <param name="_transform">Transform parameters.</param>
        inline void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position);
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }

		/// <summary>
		/// Gets the combined transform of the label.
		/// </summary>
		/// <returns>Transform structure.</returns>
        inline Transform GetTransform() const
        {
            return Transform(GetPosition(), GetRotation(), GetScale());
        }

		/// <summary>
		/// Gets the local bounding rectangle of the text without transformations.
		/// </summary>
		/// <returns>Local bounds rectangle.</returns>
        inline FRect GetLocalBounds() const
        {
            return text.GetLocalBounds();
        }

		/// <summary>
		/// Gets the global bounding rectangle of the text in world space.
		/// </summary>
		/// <returns>Global bounds rectangle.</returns>
        inline FRect GetGlobalBounds() const
        {
            return text.GetGlobalBounds();
        }

		/// <summary>
		/// Finds the 2D position coordinates of a character glyph at a specific index.
		/// </summary>
		/// <param name="_index">Index of the character glyph.</param>
		/// <returns>Position of the glyph.</returns>
        inline FVector2 FindCharacterPos(const Int& _index) const
        {
            return text.FindCharacterPos(_index);
        }

		/// <summary>
		/// Constructs a LabelComponent attached to the specified actor with text data configuration.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_data">Initial text data configuration.</param>
		LabelComponent(Actor* _owner, const TextData& _data);

	private:
		void Draw(Window& _window) override;
        void DrawDebug() override;

    public:
		/// <summary>
		/// Lifecycle callback invoked when the component is constructed.
		/// </summary>
        virtual void Construct() override;

		/// <summary>
		/// Lifecycle callback invoked when the component is deconstructed.
		/// </summary>
        virtual void Deconstruct() override;

		/// <summary>
		/// Sets the Z-order render layer for this label.
		/// </summary>
		/// <param name="_zOrder">Numeric Z-order value.</param>
        virtual void SetZOrder(const uint8_t& _zOrder) override;

		/// <summary>
		/// Sets the Z-order render layer using predefined ZOrder enumeration values.
		/// </summary>
		/// <param name="_zOrder">ZOrder enum value.</param>
        virtual void SetZOrder(const ZOrder& _zOrder) override;

    public:
		/// <summary>
		/// Returns a string representation of the component.
		/// </summary>
		/// <returns>String representation.</returns>
        virtual std::string ToString() const override;
    };

}
