#include "Color.h"

using namespace Krampus;

//Color 
constexpr Krampus::Color::Color() noexcept
{
    r = g = b = 0;
    a = 255;
}

constexpr Krampus::Color::Color(const std::uint8_t& _r, const std::uint8_t& _g, const std::uint8_t& _b, const std::uint8_t& _a) noexcept
{
    r = _r;
    g = _g;
    b = _b;
    a = _a;
}

constexpr Krampus::Color::Color(const std::uint32_t& _color) noexcept
{
    r = CAST(std::uint8_t, (_color >> 24) & 255);
    g = CAST(std::uint8_t, (_color >> 16) & 255);
    b = CAST(std::uint8_t, (_color >> 8) & 255);
    a = CAST(std::uint8_t, (_color) & 255);
}

constexpr Krampus::Color::Color(const sf::Color& _color) noexcept
{
    r = _color.r;
    g = _color.g;
    b = _color.b;
    a = _color.a;
}

constexpr std::uint32_t Krampus::Color::ToInteger() const noexcept
{
    return (CAST(std::uint32_t, r) << 24) |
        (CAST(std::uint32_t, g) << 16) |
        (CAST(std::uint32_t, b) << 8) |
        (CAST(std::uint32_t, a));
}


std::string Krampus::Color::ToString(const bool& _textOnly) const noexcept
{
    return _textOnly ? TEXT_RGB(r, g, b) : BG_RGB(r, g, b);
}


//Gradiant
Krampus::Gradient::Gradient(const Color& _a, const Color& _b)
{
    gradA = _a;
    gradB = _b;
}

std::string Gradient::GradientString(const std::string& _text, const bool& _textOnly) const
{
    const int _size = static_cast<int>(_text.size());
    std::string _newWord = "";

    for (int _index = 0; _index < _size; _index++)
    {
        const Color& _color = ClampGradient(_index, _size);
        _newWord += _color.ToString(_textOnly) + _text[_index];
    }
    return _newWord + COLOR_RESET;
}

Color Gradient::ClampGradient(const int& _index, const int& _maxDisplayChar) const
{
    float _normalizer;
    float _valueRed = 0;
    float _valueGreen = 0;
    float _valueBlue = 0;

    const float& _rangeRed = CAST(float, gradB.r - gradA.r);
    _normalizer = _index * (_rangeRed / _maxDisplayChar);
    _valueRed += (gradA.r + _normalizer);

    const float& _rangeGreen = CAST(float, gradB.g - gradA.g);
    _normalizer = _index * (_rangeGreen / _maxDisplayChar);
    _valueGreen += (gradA.g + _normalizer);

    const float& _rangeBlue = CAST(float, gradB.b - gradA.b);
    _normalizer = _index * (_rangeBlue / _maxDisplayChar);
    _valueBlue += (gradA.b + _normalizer);

    return Color(
        CAST(std::int8_t, _valueRed),
        CAST(std::int8_t, _valueGreen),
        CAST(std::int8_t, _valueBlue));
}

