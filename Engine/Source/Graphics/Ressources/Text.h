#pragma once
#include "Font.h"
#include "Utilities/Debug/Color.h"
#include "Utilities/Math/Transform.h"
#include "Managers/FontManager.h"

namespace Krampus
{

    struct TextData
    {
        String              text            = "Unknown";
        UInt                characterSize   = 30u;
        String              fontFileName    = "_internal/DefaultFont";
        FontExtensionType   extentionType   = FontExtensionType::OTF;

        TextData(const String& _text = "Unknown", const UInt& _characterSize = 30u, const String& _fontFileName = "_internal/DefaultFont", const FontExtensionType _extentionType = FontExtensionType::OTF)
        {
            text = _text;
            characterSize = _characterSize;
            fontFileName = _fontFileName;
            extentionType = _extentionType;
        }
    };

    using TextStyle = sf::Text::Style;

    class Text
    {

        sf::Text    text;
        Font        font;

    public:

        Text(const TextData& _data)
            : text(font, _data.text.StdString(), _data.characterSize)
        {
            M_FONT.LoadFont(font, _data.fontFileName, _data.extentionType);
            SetOriginAtMiddle();
        }

        inline virtual void SetString(const String& _string)
        { 
            text.setString(_string.StdString());
            SetOriginAtMiddle();
        }
        inline String GetString() const
        {
            return String(text.getString());
        }

        inline void SetFont(const String& _fileName, const FontExtensionType& _type)
        {
            M_FONT.LoadFont(font, _fileName, _type);
            SetOriginAtMiddle();
        }
        inline const Font& GetFont() const
        {
            return font;
        }

        inline void SetCharacterSize(const UInt& _size)
        {
            text.setCharacterSize(_size);
            SetOriginAtMiddle();
        }
        inline UInt GetCharacterSize() const
        {
            return text.getCharacterSize();
        }

        inline void SetFillColor(const Color& _color)
        {
            text.setFillColor(_color);
        }
        inline Color GetFillColor() const
        {
            return text.getFillColor();
        }

        inline void SetOutlineColor(const Color& _color)
        {
            text.setOutlineColor(_color);
        }
        inline Color GetOutlineColor() const
        {
            return text.getOutlineColor();
        }

        inline void SetOutlineThickness(const Float& _thickness)
        {
            text.setOutlineThickness(_thickness);
            SetOriginAtMiddle();
        }
        inline Float GetOutlineThickness() const
        {
            return text.getOutlineThickness();
        }

        inline void SetStyle(const UInt& _style)
        {
            text.setStyle(_style);
            SetOriginAtMiddle();
        }
        inline UInt GetStyle() const
        {
            return CAST(UInt, text.getStyle());
        }

        inline void SetLetterSpacing(const Float& _spacing)
        {
            text.setLetterSpacing(_spacing);
            SetOriginAtMiddle();
        }
        inline Float GetLetterSpacing() const
        {
            return text.getLetterSpacing();
        }

        inline void SetLineSpacing(const Float& _spacing)
        {
            text.setLineSpacing(_spacing);
            SetOriginAtMiddle();
        }
        inline Float GetLineSpacing() const
        { 
            return text.getLineSpacing();
        }

        inline void SetPosition(const FVector2& _position)
        {
            text.setPosition(_position);
        }
        inline FVector2 GetPosition() const
        { 
            return text.getPosition();
        }

        inline void SetOrigin(const FVector2& _origin)
        { 
            text.setOrigin(_origin);
        }
        inline void SetOriginAtMiddle()
        {
            sf::Rect<float> _bounds = text.getLocalBounds();
            SetOrigin(_bounds.position + _bounds.size / 2.0f);
        }
        inline FVector2 GetOrigin() const
        { 
            return text.getOrigin();
        }

        inline void SetScale(const FVector2& _factors)
        { 
            text.setScale(_factors);
        }
        inline FVector2 GetScale() const
        { 
            return text.getScale();
        }

        inline void SetRotation(const Angle& _angle)
        { 
            text.setRotation(_angle);
        }
        inline Angle GetRotation() const
        { 
            return Angle(text.getRotation());
        }

        inline void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position); 
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
        inline Transform GetTransform() const
        {
            return Transform(GetPosition(), GetRotation(), GetScale());
        }


        inline FRect GetLocalBounds() const
        { 
            return text.getLocalBounds();
        }
        inline FRect GetGlobalBounds() const
        { 
            return text.getGlobalBounds();
        }
        inline FVector2 FindCharacterPos(const std::size_t& _index) const
        { 
            return text.findCharacterPos(_index);
        }

        inline operator const sf::Text& () const noexcept
        { 
            return text;
        }
    };

}