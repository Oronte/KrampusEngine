#include "UInt.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void UInt::Divide(const UInt& _v)
{
    if (_v.value == 0) { LOG_ERROR("UInt::Divide: division by zero. Value set to max."); value = std::numeric_limits<unsigned int>::max(); return; }
    value /= _v.value;
}
void UInt::Modulo(const UInt& _v)
{
    if (_v.value == 0) { LOG_ERROR("UInt::Modulo: modulo by zero. Value unchanged."); return; }
    value %= _v.value;
}
UInt UInt::operator/(const UInt& o) const
{
    if (o.value == 0) { LOG_ERROR("UInt::operator/: division by zero. Returning max."); return MaxUInt(); }
    return UInt(value / o.value);
}
UInt UInt::operator%(const UInt& o) const
{
    if (o.value == 0) { LOG_ERROR("UInt::operator%: modulo by zero. Returning 0."); return UInt(0); }
    return UInt(value % o.value);
}
UInt& UInt::operator/=(const UInt& o)
{
    if (o.value == 0) { LOG_ERROR("UInt::operator/=: division by zero. Value set to max."); value = std::numeric_limits<unsigned int>::max(); return *this; }
    value /= o.value; return *this;
}
UInt& UInt::operator%=(const UInt& o)
{
    if (o.value == 0) { LOG_ERROR("UInt::operator%=: modulo by zero. Value unchanged."); return *this; }
    value %= o.value; return *this;
}
UInt UInt::Parse(const std::string& _s)
{
    try   { return UInt((unsigned int)std::stoll(_s)); }
    catch (const std::exception& e) { LOG_WARNING("UInt::Parse: cannot parse \"" + _s + "\": " + e.what()); return UInt(0); }
}
UInt UInt::Clamp(const UInt& v, const UInt& mn, const UInt& mx)
{
    if (mn.value > mx.value) LOG_WARNING("UInt::Clamp: min > max.");
    return UInt(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
