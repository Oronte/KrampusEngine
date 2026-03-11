#include "Int.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void Int::Abs()    { value = (int)std::abs(value); }
void Int::Negate() { value = -(int)std::abs(value); }
void Int::Invert() { value *= (int)-1; }
Int Int::Abs(const Int& v) { return Int((int)std::abs(v.value)); }

void Int::Divide(const Int& _v)
{
    if (_v.value == 0) { LOG_ERROR("Int::Divide: division by zero. Value set to max."); value = std::numeric_limits<int>::max(); return; }
    value /= _v.value;
}
void Int::Modulo(const Int& _v)
{
    if (_v.value == 0) { LOG_ERROR("Int::Modulo: modulo by zero. Value unchanged."); return; }
    value %= _v.value;
}
Int Int::operator/(const Int& o) const
{
    if (o.value == 0) { LOG_ERROR("Int::operator/: division by zero. Returning max."); return MaxInt(); }
    return Int(value / o.value);
}
Int Int::operator%(const Int& o) const
{
    if (o.value == 0) { LOG_ERROR("Int::operator%: modulo by zero. Returning 0."); return Int(0); }
    return Int(value % o.value);
}
Int& Int::operator/=(const Int& o)
{
    if (o.value == 0) { LOG_ERROR("Int::operator/=: division by zero. Value set to max."); value = std::numeric_limits<int>::max(); return *this; }
    value /= o.value; return *this;
}
Int& Int::operator%=(const Int& o)
{
    if (o.value == 0) { LOG_ERROR("Int::operator%=: modulo by zero. Value unchanged."); return *this; }
    value %= o.value; return *this;
}
Int Int::Parse(const std::string& _s)
{
    try   { return Int((int)std::stoll(_s)); }
    catch (const std::exception& e) { LOG_WARNING("Int::Parse: cannot parse \"" + _s + "\": " + e.what()); return Int(0); }
}
Int Int::Clamp(const Int& v, const Int& mn, const Int& mx)
{
    if (mn.value > mx.value) LOG_WARNING("Int::Clamp: min > max.");
    return Int(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
