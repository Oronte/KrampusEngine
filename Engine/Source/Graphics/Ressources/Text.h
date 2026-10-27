#pragma once
#include "Font.h"
#include "Utilities/Debug/Color.h"
#include "Utilities/Math/Transform.h"
#include "Managers/FontManager.h"

namespace Krampus
{

    /// <summary>
    /// Configuration data used to initialize text strings, character size, and font resources.
    /// </summary>
    struct TextData
    {
        String              text            = "Unknown";
        UInt                characterSize   = 30u;
        String              fontFileName    = "_internal/DefaultFont";
        FontExtensionType   extentionType   = FontExtensionType::OTF;

        /// <summary>
        /// Constructs TextData with optional parameters.
        /// </summary>
        /// <param name="_text">Initial text string.</param>
        /// <param name="_characterSize">Font size in pixels.</param>
        /// <param name="_fontFileName">Font resource name or path.</param>
        /// <param name="_extentionType">Font file extension type.</param>
        TextData(const String& _text = "Unknown", const UInt& _characterSize = 30u, const String& _fontFileName = "_internal/DefaultFont", const FontExtensionType _extentionType = FontExtensionType::OTF)
        {
            text = _text;
            characterSize = _characterSize;
            fontFileName = _fontFileName;
            extentionType = _extentionType;
        }
    };

    /// <summary>
    /// Type alias for SFML text styling flags (Regular, Bold, Italic, Underlined, StrikeThrough).
    /// </summary>
    using TextStyle = sf::Text::Style;

    /// <summary>
    /// Encapsulates text rendering primitives, styling, typography, and 2D transformations.
    /// </summary>
    class Text
    {

        sf::Text    text;
        Font        font;

    public:

        /// <summary>
        /// Constructs a Text instance initialized with text configuration data.
        /// </summary>
        /// <param name="_data">TextData containing string, size, and font settings.</param>
        Text(const TextData& _data)
            : text(font, _data.text.StdString(), _data.characterSize)
        {
            M_FONT.LoadFont(font, _data.fontFileName, _data.extentionType);
            SetOriginAtMiddle();
        }

        /// <summary>
        /// Sets the displayed text string and recalculates origin to center.
        /// </summary>
        /// <param name="_string">String content.</param>
        inline virtual void SetString(const String& _string)
        { 
            text.setString(_string.StdString());
            SetOriginAtMiddle();
        }

        /// <summary>
        /// Gets the current displayed text string.
        /// </summary>
        /// <returns>Text string.</returns>
        inline String GetString() const
        {
            return String(text.getString());
        }

        /// <summary>
        /// Changes the font resource used by this text.
        /// </summary>
        /// <param name="_fileName">Font resource name.</param>
        /// <param name="_type">Font extension type.</param>
        inline void SetFont(const String& _fileName, const FontExtensionType& _type)
        {
            M_FONT.LoadFont(font, _fileName, _type);
            SetOriginAtMiddle();
        }

        /// <summary>
        /// Gets a const reference to the font used by this text.
        /// </summary>
        /// <returns>Reference to Font.</returns>
        inline const Font& GetFont() const
        {
            return font;
        }

        /// <summary>
        /// Sets the character size in pixels and centers the origin.
        /// </summary>
        /// <param name="_size">Character size in pixels.</param>
        inline void SetCharacterSize(const UInt& _size)
        {
            text.setCharacterSize(_size);
            SetOriginAtMiddle();
        }

        /// <summary>
        /// Gets the character size in pixels.
        /// </summary>
        /// <returns>Character size in pixels.</returns>
        inline UInt GetCharacterSize() const
        {
            return text.getCharacterSize();
        }

        /// <summary>
        /// Sets the fill color of the text glyphs.
        /// </summary>
        /// <param name="_color">Fill color.</param>
        inline void SetFillColor(const Color& _color)
        {
            text.setFillColor(_color);
        }

        /// <summary>
        /// Gets the fill color of the text glyphs.
        /// </summary>
        /// <returns>Fill color.</returns>
        inline Color GetFillColor() const
        {
            return text.getFillColor();
        }

        /// <summary>
        /// Sets the outline color of the text glyphs.
        /// </summary>
        /// <param name="_color">Outline color.</param>
        inline void SetOutlineColor(const Color& _color)
        {
            text.setOutlineColor(_color);
        }

        /// <summary>
        /// Gets the outline color of the text glyphs.
        /// </summary>
        /// <returns>Outline color.</returns>
        inline Color GetOutlineColor() const
        {
            return text.getOutlineColor();
        }

        /// <summary>
        /// Sets the outline thickness in pixels and centers the origin.
        /// </summary>
        /// <param name="_thickness">Outline thickness.</param>
        inline void SetOutlineThickness(const Float& _thickness)
        {
            text.setOutlineThickness(_thickness);
            SetOriginAtMiddle();
        }

        /// <summary>
        /// Gets the outline thickness in pixels.
        /// </summary>
        /// <returns>Outline thickness.</returns>
        inline Float GetOutlineThickness() const
        {
            return text.getOutlineThickness();
        }

        /// <summary>
        /// Sets text styling flags (Bold, Italic, Underlined, StrikeThrough).
        /// </summary>
        /// <param name="_style">Style bitmask flags.</param>
        inline void SetStyle(const UInt& _style)
        {
            text.setStyle(_style);
            SetOriginAtMiddle();
        }

        /// <summary>
        /// Gets the text styling bitmask flags.
        /// </summary>
        /// <returns>Style flags.</returns>
        inline UInt GetStyle() const
        {
            return CAST(UInt, text.getStyle());
        }

        /// <summary>
        /// Sets the spacing factor between characters and centers the origin.
        /// </summary>
        /// <param name="_spacing">Letter spacing factor.</param>
        inline void SetLetterSpacing(const Float& _spacing)
        {
            text.setLetterSpacing(_spacing);
            SetOriginAtMiddle();
        }

        /// <summary>
        /// Gets the letter spacing factor.
        /// </summary>
        /// <returns>Letter spacing factor.</returns>
        inline Float GetLetterSpacing() const
        {
            return text.getLetterSpacing();
        }

        /// <summary>
        /// Sets the line spacing factor and centers the origin.
        /// </summary>
        /// <param name="_spacing">Line spacing factor.</param>
        inline void SetLineSpacing(const Float& _spacing)
        {
            text.setLineSpacing(_spacing);
            SetOriginAtMiddle();
        }

        /// <summary>
        /// Gets the line spacing factor.
        /// </summary>
        /// <returns>Line spacing factor.</returns>
        inline Float GetLineSpacing() const
        { 
            return text.getLineSpacing();
        }

        /// <summary>
        /// Sets the position of the text in 2D space.
        /// </summary>
        /// <param name="_position">2D position vector.</param>
        inline void SetPosition(const FVector2& _position)
        {
            text.setPosition(_position);
        }

        /// <summary>
        /// Gets the position of the text in 2D space.
        /// </summary>
        /// <returns>2D position vector.</returns>
        inline FVector2 GetPosition() const
        { 
            return text.getPosition();
        }

        /// <summary>
        /// Sets the origin point of the text for transformations.
        /// </summary>
        /// <param name="_origin">Origin coordinates.</param>
        inline void SetOrigin(const FVector2& _origin)
        { 
            text.setOrigin(_origin);
        }

        /// <summary>
        /// Automatically centers the text origin at the geometric middle of its local bounds.
        /// </summary>
        inline void SetOriginAtMiddle()
        {
            sf::Rect<float> _bounds = text.getLocalBounds();
            SetOrigin(_bounds.position + _bounds.size / 2.0f);
        }

        /// <summary>
        /// Gets the origin point of the text.
        /// </summary>
        /// <returns>Origin coordinates.</returns>
        inline FVector2 GetOrigin() const
        { 
            return text.getOrigin();
        }

        /// <summary>
        /// Sets the 2D scale factor of the text.
        /// </summary>
        /// <param name="_factors">Scale multipliers.</param>
        inline void SetScale(const FVector2& _factors)
        { 
            text.setScale(_factors);
        }

        /// <summary>
        /// Gets the 2D scale factor of the text.
        /// </summary>
        /// <returns>Scale multipliers.</returns>
        inline FVector2 GetScale() const
        { 
            return text.getScale();
        }

        /// <summary>
        /// Sets the rotation angle of the text.
        /// </summary>
        /// <param name="_angle">Rotation angle.</param>
        inline void SetRotation(const Angle& _angle)
        { 
            text.setRotation(_angle);
        }

        /// <summary>
        /// Gets the rotation angle of the text.
        /// </summary>
        /// <returns>Rotation angle.</returns>
        inline Angle GetRotation() const
        { 
            return Angle(text.getRotation());
        }

        /// <summary>
        /// Sets the combined transform (position, rotation, scale) of the text.
        /// </summary>
        /// <param name="_transform">Transform parameters.</param>
        inline void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position); 
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }

        /// <summary>
        /// Gets the combined transform of the text.
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
            return text.getLocalBounds();
        }

        /// <summary>
        /// Gets the global bounding rectangle of the text in world space.
        /// </summary>
        /// <returns>Global bounds rectangle.</returns>
        inline FRect GetGlobalBounds() const
        { 
            return text.getGlobalBounds();
        }

        /// <summary>
        /// Finds the 2D position of a character glyph at the specified index.
        /// </summary>
        /// <param name="_index">Index of the character glyph.</param>
        /// <returns>Position coordinates.</returns>
        inline FVector2 FindCharacterPos(const std::size_t& _index) const
        { 
            return text.findCharacterPos(_index);
        }

        /// <summary>
        /// Implicit conversion to const reference of the underlying sf::Text.
        /// </summary>
        /// <returns>Reference to internal sf::Text.</returns>
        inline operator const sf::Text& () const noexcept
        { 
            return text;
        }
    };

}
