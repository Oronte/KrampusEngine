#include "LongLong.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void LongLong::Abs()    { value = (long long)std::abs(value); }
void LongLong::Negate() { value = -(long long)std::abs(value); }
void LongLong::Invert() { value *= (long long)-1; }
LongLong LongLong::Abs(const LongLong& v) { return LongLong((long long)std::abs(v.value)); }

void LongLong::Divide(const LongLong& _v)
{
    if (_v.value == 0) { LOG_ERROR("LongLong::Divide: division by zero. Value set to max."); value = std::numeric_limits<long long>::max(); return; }
    value /= _v.value;
}
void LongLong::Modulo(const LongLong& _v)
{
    if (_v.value == 0) { LOG_ERROR("LongLong::Modulo: modulo by zero. Value unchanged."); return; }
    value %= _v.value;
}
LongLong LongLong::operator/(const LongLong& o) const
{
    if (o.value == 0) { LOG_ERROR("LongLong::operator/: division by zero. Returning max."); return MaxLongLong(); }
    return LongLong(value / o.value);
}
LongLong LongLong::operator%(const LongLong& o) const
{
    if (o.value == 0) { LOG_ERROR("LongLong::operator%: modulo by zero. Returning 0."); return LongLong(0); }
    return LongLong(value % o.value);
}
LongLong& LongLong::operator/=(const LongLong& o)
{
    if (o.value == 0) { LOG_ERROR("LongLong::operator/=: division by zero. Value set to max."); value = std::numeric_limits<long long>::max(); return *this; }
    value /= o.value; return *this;
}
LongLong& LongLong::operator%=(const LongLong& o)
{
    if (o.value == 0) { LOG_ERROR("LongLong::operator%=: modulo by zero. Value unchanged."); return *this; }
    value %= o.value; return *this;
}
LongLong LongLong::Parse(const std::string& _s)
{
    try   { return LongLong((long long)std::stoll(_s)); }
    catch (const std::exception& e) { LOG_WARNING("LongLong::Parse: cannot parse \"" + _s + "\": " + e.what()); return LongLong(0); }
}
LongLong LongLong::Clamp(const LongLong& v, const LongLong& mn, const LongLong& mx)
{
    if (mn.value > mx.value) LOG_WARNING("LongLong::Clamp: min > max.");
    return LongLong(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
