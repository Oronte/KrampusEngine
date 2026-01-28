#pragma once
#include "Font.h"
#include "Utilities/Debug/Color.h"
#include "Utilities/Math/Transform.h"
#include "Managers/FontManager.h"

namespace Krampus
{

    struct TextData
    {
        std::string text = "Unknown";
        unsigned int characterSize = 30u;
        std::string fontFileName = "DefaultFont";
        FontExtensionType extentionType = FontExtensionType::OTF;
    };

    using TextStyle = sf::Text::Style;

    class Text
    {

        sf::Text text;
        Font font;

    public:

        Text(const TextData& _data)
            : text(font, _data.text, _data.characterSize)
        {
            M_FONT.LoadFont(font, _data.fontFileName, _data.extentionType);
            SetOriginAtMiddle();
        }

        inline virtual void SetString(const std::string& _string)
        { 
            text.setString(_string);
            SetOriginAtMiddle();
        }
        inline const std::string& GetString() const
        {
            return text.getString();
        }

        inline void SetFont(const std::string& _fileName, const FontExtensionType& _type)
        {
            M_FONT.LoadFont(font, _fileName, _type);
            SetOriginAtMiddle();
        }
        inline const Font& GetFont() const
        {
            return font;
        }

        inline void SetCharacterSize(unsigned int& _size)
        {
            text.setCharacterSize(_size);
            SetOriginAtMiddle();
        }
        inline unsigned int GetCharacterSize() const
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

        inline void SetOutlineThickness(const float& _thickness)
        {
            text.setOutlineThickness(_thickness);
            SetOriginAtMiddle();
        }
        inline float GetOutlineThickness() const
        {
            return text.getOutlineThickness();
        }

        inline void SetStyle(const unsigned int& _style)
        {
            text.setStyle(_style);
            SetOriginAtMiddle();
        }
        inline unsigned int GetStyle() const
        {
            return CAST(TextStyle, text.getStyle());
        }

        inline void SetLetterSpacing(const float& _spacing)
        {
            text.setLetterSpacing(_spacing);
            SetOriginAtMiddle();
        }
        inline float GetLetterSpacing() const
        {
            return text.getLetterSpacing();
        }

        inline void SetLineSpacing(const float& _spacing)
        {
            text.setLineSpacing(_spacing);
            SetOriginAtMiddle();
        }
        inline float GetLineSpacing() const
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