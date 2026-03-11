#include "Byte.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void Byte::Divide(const Byte& _v)
{
    if (_v.value == 0) { LOG_ERROR("Byte::Divide: division by zero. Value set to max."); value = std::numeric_limits<unsigned char>::max(); return; }
    value /= _v.value;
}
void Byte::Modulo(const Byte& _v)
{
    if (_v.value == 0) { LOG_ERROR("Byte::Modulo: modulo by zero. Value unchanged."); return; }
    value %= _v.value;
}
Byte Byte::operator/(const Byte& o) const
{
    if (o.value == 0) { LOG_ERROR("Byte::operator/: division by zero. Returning max."); return MaxByte(); }
    return Byte(value / o.value);
}
Byte Byte::operator%(const Byte& o) const
{
    if (o.value == 0) { LOG_ERROR("Byte::operator%: modulo by zero. Returning 0."); return Byte(0); }
    return Byte(value % o.value);
}
Byte& Byte::operator/=(const Byte& o)
{
    if (o.value == 0) { LOG_ERROR("Byte::operator/=: division by zero. Value set to max."); value = std::numeric_limits<unsigned char>::max(); return *this; }
    value /= o.value; return *this;
}
Byte& Byte::operator%=(const Byte& o)
{
    if (o.value == 0) { LOG_ERROR("Byte::operator%=: modulo by zero. Value unchanged."); return *this; }
    value %= o.value; return *this;
}
Byte Byte::Parse(const std::string& _s)
{
    try   { return Byte((unsigned char)std::stoll(_s)); }
    catch (const std::exception& e) { LOG_WARNING("Byte::Parse: cannot parse \"" + _s + "\": " + e.what()); return Byte(0); }
}
Byte Byte::Clamp(const Byte& v, const Byte& mn, const Byte& mx)
{
    if (mn.value > mx.value) LOG_WARNING("Byte::Clamp: min > max.");
    return Byte(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
