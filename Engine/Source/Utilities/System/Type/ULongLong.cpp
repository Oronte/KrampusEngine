#include "ULongLong.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void ULongLong::Divide(const ULongLong& _v)
{
    if (_v.value == 0) { LOG_ERROR("ULongLong::Divide: division by zero. Value set to max."); value = std::numeric_limits<unsigned long long>::max(); return; }
    value /= _v.value;
}
void ULongLong::Modulo(const ULongLong& _v)
{
    if (_v.value == 0) { LOG_ERROR("ULongLong::Modulo: modulo by zero. Value unchanged."); return; }
    value %= _v.value;
}
ULongLong ULongLong::operator/(const ULongLong& o) const
{
    if (o.value == 0) { LOG_ERROR("ULongLong::operator/: division by zero. Returning max."); return MaxULongLong(); }
    return ULongLong(value / o.value);
}
ULongLong ULongLong::operator%(const ULongLong& o) const
{
    if (o.value == 0) { LOG_ERROR("ULongLong::operator%: modulo by zero. Returning 0."); return ULongLong(0); }
    return ULongLong(value % o.value);
}
ULongLong& ULongLong::operator/=(const ULongLong& o)
{
    if (o.value == 0) { LOG_ERROR("ULongLong::operator/=: division by zero. Value set to max."); value = std::numeric_limits<unsigned long long>::max(); return *this; }
    value /= o.value; return *this;
}
ULongLong& ULongLong::operator%=(const ULongLong& o)
{
    if (o.value == 0) { LOG_ERROR("ULongLong::operator%=: modulo by zero. Value unchanged."); return *this; }
    value %= o.value; return *this;
}
ULongLong ULongLong::Parse(const std::string& _s)
{
    try   { return ULongLong((unsigned long long)std::stoll(_s)); }
    catch (const std::exception& e) { LOG_WARNING("ULongLong::Parse: cannot parse \"" + _s + "\": " + e.what()); return ULongLong(0); }
}
ULongLong ULongLong::Clamp(const ULongLong& v, const ULongLong& mn, const ULongLong& mx)
{
    if (mn.value > mx.value) LOG_WARNING("ULongLong::Clamp: min > max.");
    return ULongLong(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
