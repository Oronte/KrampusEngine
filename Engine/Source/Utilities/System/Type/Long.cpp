#include "Long.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void Long::Abs()    { value = (long)std::abs(value); }
void Long::Negate() { value = -(long)std::abs(value); }
void Long::Invert() { value *= (long)-1; }
Long Long::Abs(const Long& v) { return Long((long)std::abs(v.value)); }

void Long::Divide(const Long& _v)
{
    if (_v.value == 0) { LOG_ERROR("Long::Divide: division by zero. Value set to max."); value = std::numeric_limits<long>::max(); return; }
    value /= _v.value;
}
void Long::Modulo(const Long& _v)
{
    if (_v.value == 0) { LOG_ERROR("Long::Modulo: modulo by zero. Value unchanged."); return; }
    value %= _v.value;
}
Long Long::operator/(const Long& o) const
{
    if (o.value == 0) { LOG_ERROR("Long::operator/: division by zero. Returning max."); return MaxLong(); }
    return Long(value / o.value);
}
Long Long::operator%(const Long& o) const
{
    if (o.value == 0) { LOG_ERROR("Long::operator%: modulo by zero. Returning 0."); return Long(0); }
    return Long(value % o.value);
}
Long& Long::operator/=(const Long& o)
{
    if (o.value == 0) { LOG_ERROR("Long::operator/=: division by zero. Value set to max."); value = std::numeric_limits<long>::max(); return *this; }
    value /= o.value; return *this;
}
Long& Long::operator%=(const Long& o)
{
    if (o.value == 0) { LOG_ERROR("Long::operator%=: modulo by zero. Value unchanged."); return *this; }
    value %= o.value; return *this;
}
Long Long::Parse(const std::string& _s)
{
    try   { return Long((long)std::stoll(_s)); }
    catch (const std::exception& e) { LOG_WARNING("Long::Parse: cannot parse \"" + _s + "\": " + e.what()); return Long(0); }
}
Long Long::Clamp(const Long& v, const Long& mn, const Long& mx)
{
    if (mn.value > mx.value) LOG_WARNING("Long::Clamp: min > max.");
    return Long(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
