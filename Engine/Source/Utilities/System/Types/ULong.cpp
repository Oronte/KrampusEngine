#include "ULong.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void ULong::Divide(const ULong& _v)
{
    if (_v.value == 0) { LOG_ERROR("ULong::Divide: division by zero. Value set to max."); value = std::numeric_limits<unsigned long>::max(); return; }
    value /= _v.value;
}
void ULong::Modulo(const ULong& _v)
{
    if (_v.value == 0) { LOG_ERROR("ULong::Modulo: modulo by zero. Value unchanged."); return; }
    value %= _v.value;
}
ULong ULong::operator/(const ULong& o) const
{
    if (o.value == 0) { LOG_ERROR("ULong::operator/: division by zero. Returning max."); return MaxULong(); }
    return ULong(value / o.value);
}
ULong ULong::operator%(const ULong& o) const
{
    if (o.value == 0) { LOG_ERROR("ULong::operator%: modulo by zero. Returning 0."); return ULong(0); }
    return ULong(value % o.value);
}
ULong& ULong::operator/=(const ULong& o)
{
    if (o.value == 0) { LOG_ERROR("ULong::operator/=: division by zero. Value set to max."); value = std::numeric_limits<unsigned long>::max(); return *this; }
    value /= o.value; return *this;
}
ULong& ULong::operator%=(const ULong& o)
{
    if (o.value == 0) { LOG_ERROR("ULong::operator%=: modulo by zero. Value unchanged."); return *this; }
    value %= o.value; return *this;
}
ULong ULong::Parse(const std::string& _s)
{
    try   { return ULong((unsigned long)std::stoll(_s)); }
    catch (const std::exception& e) { LOG_WARNING("ULong::Parse: cannot parse \"" + _s + "\": " + e.what()); return ULong(0); }
}
ULong ULong::Clamp(const ULong& v, const ULong& mn, const ULong& mx)
{
    if (mn.value > mx.value) LOG_WARNING("ULong::Clamp: min > max.");
    return ULong(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
