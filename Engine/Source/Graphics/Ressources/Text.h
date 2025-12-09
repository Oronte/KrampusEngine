#pragma once
#include "Font.h"
#include "Utilities/Color.h"
#include "Utilities/Math/Transform.h"
#include "Managers/FontManager.h"

namespace Krampus
{

    class Text
    {
        sf::Text text;
        Font font;

    public:
        Text(const std::string& _string = "", unsigned int _characterSize = 30u, const std::string& _fontFileName = "DefaultFont", const FontExtensionType& _type = FontExtensionType::OTF)
            : text(font, _string, _characterSize)
        {
            M_FONT.LoadFont(font, _fontFileName, _type);
        }

        INLINE void SetString(const std::string& _string) 
        { 
            text.setString(_string);
        }
        INLINE const std::string& GetString() const 
        { 
            return text.getString();
        }

        INLINE void SetFont(const std::string& _fileName, const FontExtensionType& _type)
        { 
            M_FONT.LoadFont(font, _fileName, _type);
        }
        INLINE const Font& GetFont() const 
        { 
            return font;
        }

        INLINE void SetCharacterSize(unsigned int& _size) 
        { 
            text.setCharacterSize(_size);
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
        }
        INLINE float GetOutlineThickness() const 
        {
            return text.getOutlineThickness();
        }

        INLINE void SetStyle(const unsigned int& _style) 
        { 
            text.setStyle(_style);
        }
        INLINE unsigned int GetStyle() const 
        { 
            return text.getStyle();
        }

        INLINE void SetLetterSpacing(const float& _spacing) 
        {
            text.setLetterSpacing(_spacing);
        }
        INLINE float GetLetterSpacing() const 
        { 
            return text.getLetterSpacing();
        }

        INLINE void SetLineSpacing(const float& _spacing) 
        { 
            text.setLineSpacing(_spacing);
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
            SetOrigin(_transform.origin);
            SetPosition(_transform.position); 
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
        INLINE Transform GetTransform() const
        {
            return Transform(GetOrigin(), GetPosition(), GetRotation(), GetScale());
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