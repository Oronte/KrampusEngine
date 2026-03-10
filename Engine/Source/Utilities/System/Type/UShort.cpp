#include "UShort.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void UShort::Divide(const UShort& _v)
{
    if (_v.value == 0) { LOG_ERROR("UShort::Divide: division by zero. Value set to max."); value = std::numeric_limits<unsigned short>::max(); return; }
    value /= _v.value;
}
void UShort::Modulo(const UShort& _v)
{
    if (_v.value == 0) { LOG_ERROR("UShort::Modulo: modulo by zero. Value unchanged."); return; }
    value %= _v.value;
}
UShort UShort::operator/(const UShort& o) const
{
    if (o.value == 0) { LOG_ERROR("UShort::operator/: division by zero. Returning max."); return MaxUShort(); }
    return UShort(value / o.value);
}
UShort UShort::operator%(const UShort& o) const
{
    if (o.value == 0) { LOG_ERROR("UShort::operator%: modulo by zero. Returning 0."); return UShort(0); }
    return UShort(value % o.value);
}
UShort& UShort::operator/=(const UShort& o)
{
    if (o.value == 0) { LOG_ERROR("UShort::operator/=: division by zero. Value set to max."); value = std::numeric_limits<unsigned short>::max(); return *this; }
    value /= o.value; return *this;
}
UShort& UShort::operator%=(const UShort& o)
{
    if (o.value == 0) { LOG_ERROR("UShort::operator%=: modulo by zero. Value unchanged."); return *this; }
    value %= o.value; return *this;
}
UShort UShort::Parse(const std::string& _s)
{
    try   { return UShort((unsigned short)std::stoll(_s)); }
    catch (const std::exception& e) { LOG_WARNING("UShort::Parse: cannot parse \"" + _s + "\": " + e.what()); return UShort(0); }
}
UShort UShort::Clamp(const UShort& v, const UShort& mn, const UShort& mx)
{
    if (mn.value > mx.value) LOG_WARNING("UShort::Clamp: min > max.");
    return UShort(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
