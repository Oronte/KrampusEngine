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

        inlin virtual void SetString(const std::string& _string)
        { 
            text.setString(_string);
            SetOriginAtMiddle();
        }
        inlin const std::string& GetString() const
        {
            return text.getString();
        }

        inlin void SetFont(const std::string& _fileName, const FontExtensionType& _type)
        {
            M_FONT.LoadFont(font, _fileName, _type);
            SetOriginAtMiddle();
        }
        inlin const Font& GetFont() const
        {
            return font;
        }

        inlin void SetCharacterSize(unsigned int& _size)
        {
            text.setCharacterSize(_size);
            SetOriginAtMiddle();
        }
        inlin unsigned int GetCharacterSize() const
        {
            return text.getCharacterSize();
        }

        inlin void SetFillColor(const Color& _color)
        {
            text.setFillColor(_color);
        }
        inlin Color GetFillColor() const
        {
            return text.getFillColor();
        }

        inlin void SetOutlineColor(const Color& _color)
        {
            text.setOutlineColor(_color);
        }
        inlin Color GetOutlineColor() const
        {
            return text.getOutlineColor();
        }

        inlin void SetOutlineThickness(const float& _thickness)
        {
            text.setOutlineThickness(_thickness);
            SetOriginAtMiddle();
        }
        inlin float GetOutlineThickness() const
        {
            return text.getOutlineThickness();
        }

        inlin void SetStyle(const unsigned int& _style)
        {
            text.setStyle(_style);
            SetOriginAtMiddle();
        }
        inlin unsigned int GetStyle() const
        {
            return CAST(TextStyle, text.getStyle());
        }

        inlin void SetLetterSpacing(const float& _spacing)
        {
            text.setLetterSpacing(_spacing);
            SetOriginAtMiddle();
        }
        inlin float GetLetterSpacing() const
        {
            return text.getLetterSpacing();
        }

        inlin void SetLineSpacing(const float& _spacing)
        {
            text.setLineSpacing(_spacing);
            SetOriginAtMiddle();
        }
        inlin float GetLineSpacing() const 
        { 
            return text.getLineSpacing();
        }

        inlin void SetPosition(const FVector2& _position) 
        {
            text.setPosition(_position);
        }
        inlin FVector2 GetPosition() const 
        { 
            return text.getPosition();
        }

        inlin void SetOrigin(const FVector2& _origin) 
        { 
            text.setOrigin(_origin);
        }
        inlin void SetOriginAtMiddle()
        {
            sf::Rect<float> _bounds = text.getLocalBounds();
            SetOrigin(_bounds.position + _bounds.size / 2.0f);
        }
        inlin FVector2 GetOrigin() const 
        { 
            return text.getOrigin();
        }

        inlin void SetScale(const FVector2& _factors) 
        { 
            text.setScale(_factors);
        }
        inlin FVector2 GetScale() const 
        { 
            return text.getScale();
        }

        inlin void SetRotation(const Angle& _angle) 
        { 
            text.setRotation(_angle);
        }
        inlin Angle GetRotation() const 
        { 
            return Angle(text.getRotation());
        }

        inlin void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position); 
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
        inlin Transform GetTransform() const
        {
            return Transform(GetPosition(), GetRotation(), GetScale());
        }


        inlin FRect GetLocalBounds() const 
        { 
            return text.getLocalBounds();
        }
        inlin FRect GetGlobalBounds() const 
        { 
            return text.getGlobalBounds();
        }
        inlin FVector2 FindCharacterPos(const std::size_t& _index) const 
        { 
            return text.findCharacterPos(_index);
        }

        inlin operator const sf::Text& () const noexcept 
        { 
            return text;
        }
    };

}