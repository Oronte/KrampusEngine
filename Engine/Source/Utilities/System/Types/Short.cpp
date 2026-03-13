#include "Short.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void Short::Abs()    { value = (short)std::abs(value); }
void Short::Negate() { value = -(short)std::abs(value); }
void Short::Invert() { value *= (short)-1; }
Short Short::Abs(const Short& v) { return Short((short)std::abs(v.value)); }

void Short::Divide(const Short& _v)
{
    if (_v.value == 0) { LOG_ERROR("Short::Divide: division by zero. Value set to max."); value = std::numeric_limits<short>::max(); return; }
    value /= _v.value;
}
void Short::Modulo(const Short& _v)
{
    if (_v.value == 0) { LOG_ERROR("Short::Modulo: modulo by zero. Value unchanged."); return; }
    value %= _v.value;
}
Short Short::operator/(const Short& o) const
{
    if (o.value == 0) { LOG_ERROR("Short::operator/: division by zero. Returning max."); return MaxShort(); }
    return Short(value / o.value);
}
Short Short::operator%(const Short& o) const
{
    if (o.value == 0) { LOG_ERROR("Short::operator%: modulo by zero. Returning 0."); return Short(0); }
    return Short(value % o.value);
}
Short& Short::operator/=(const Short& o)
{
    if (o.value == 0) { LOG_ERROR("Short::operator/=: division by zero. Value set to max."); value = std::numeric_limits<short>::max(); return *this; }
    value /= o.value; return *this;
}
Short& Short::operator%=(const Short& o)
{
    if (o.value == 0) { LOG_ERROR("Short::operator%=: modulo by zero. Value unchanged."); return *this; }
    value %= o.value; return *this;
}
Short Short::Parse(const std::string& _s)
{
    try   { return Short((short)std::stoll(_s)); }
    catch (const std::exception& e) { LOG_WARNING("Short::Parse: cannot parse \"" + _s + "\": " + e.what()); return Short(0); }
}
Short Short::Clamp(const Short& v, const Short& mn, const Short& mx)
{
    if (mn.value > mx.value) LOG_WARNING("Short::Clamp: min > max.");
    return Short(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
