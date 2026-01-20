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

        INLINE virtual void SetString(const std::string& _string)
        { 
            text.setString(_string);
            SetOriginAtMiddle();
        }
        INLINE const std::string& GetString() const
        {
            return text.getString();
        }

        INLINE void SetFont(const std::string& _fileName, const FontExtensionType& _type)
        {
            M_FONT.LoadFont(font, _fileName, _type);
            SetOriginAtMiddle();
        }
        INLINE const Font& GetFont() const
        {
            return font;
        }

        INLINE void SetCharacterSize(unsigned int& _size)
        {
            text.setCharacterSize(_size);
            SetOriginAtMiddle();
        }
        INLINE unsigned int GetCharacterSize() const
        {
            return text.getCharacterSize();
        }

        INLINE void SetFillColor(const Color& _color)
        {
            text.setFillColor(_color);
        }
        INLINE Color GetFillColor() const
        {
            return text.getFillColor();
        }

        INLINE void SetOutlineColor(const Color& _color)
        {
            text.setOutlineColor(_color);
        }
        INLINE Color GetOutlineColor() const
        {
            return text.getOutlineColor();
        }

        INLINE void SetOutlineThickness(const float& _thickness)
        {
            text.setOutlineThickness(_thickness);
            SetOriginAtMiddle();
        }
        INLINE float GetOutlineThickness() const
        {
            return text.getOutlineThickness();
        }

        INLINE void SetStyle(const unsigned int& _style)
        {
            text.setStyle(_style);
            SetOriginAtMiddle();
        }
        INLINE unsigned int GetStyle() const
        {
            return CAST(TextStyle, text.getStyle());
        }

        INLINE void SetLetterSpacing(const float& _spacing)
        {
            text.setLetterSpacing(_spacing);
            SetOriginAtMiddle();
        }
        INLINE float GetLetterSpacing() const
        {
            return text.getLetterSpacing();
        }

        INLINE void SetLineSpacing(const float& _spacing)
        {
            text.setLineSpacing(_spacing);
            SetOriginAtMiddle();
        }
        INLINE float GetLineSpacing() const 
        { 
            return text.getLineSpacing();
        }

        INLINE void SetPosition(const FVector2& _position) 
        {
            text.setPosition(_position);
        }
        INLINE FVector2 GetPosition() const 
        { 
            return text.getPosition();
        }

        INLINE void SetOrigin(const FVector2& _origin) 
        { 
            text.setOrigin(_origin);
        }
        INLINE void SetOriginAtMiddle()
        {
            sf::Rect<float> _bounds = text.getLocalBounds();
            SetOrigin(_bounds.position + _bounds.size / 2.0f);
        }
        INLINE FVector2 GetOrigin() const 
        { 
            return text.getOrigin();
        }

        INLINE void SetScale(const FVector2& _factors) 
        { 
            text.setScale(_factors);
        }
        INLINE FVector2 GetScale() const 
        { 
            return text.getScale();
        }

        INLINE void SetRotation(const Angle& _angle) 
        { 
            text.setRotation(_angle);
        }
        INLINE Angle GetRotation() const 
        { 
            return Angle(text.getRotation());
        }

        INLINE void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position); 
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
        INLINE Transform GetTransform() const
        {
            return Transform(GetPosition(), GetRotation(), GetScale());
        }


        INLINE FRect GetLocalBounds() const 
        { 
            return text.getLocalBounds();
        }
        INLINE FRect GetGlobalBounds() const 
        { 
            return text.getGlobalBounds();
        }
        INLINE FVector2 FindCharacterPos(const std::size_t& _index) const 
        { 
            return text.findCharacterPos(_index);
        }

        INLINE operator const sf::Text& () const noexcept 
        { 
            return text;
        }
    };

}