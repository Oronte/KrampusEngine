#include "LongDouble.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void LongDouble::Divide(const LongDouble& _v)
{
    if (_v.value == (long double)0) { LOG_ERROR("LongDouble::Divide: division by zero. Value set to max."); value = std::numeric_limits<long double>::max(); return; }
    value /= _v.value;
}
LongDouble LongDouble::operator/(const LongDouble& o) const
{
    if (o.value == (long double)0) { LOG_ERROR("LongDouble::operator/: division by zero. Returning max."); return MaxLongDouble(); }
    return LongDouble(value / o.value);
}
LongDouble& LongDouble::operator/=(const LongDouble& o)
{
    if (o.value == (long double)0) { LOG_ERROR("LongDouble::operator/=: division by zero. Value set to max."); value = std::numeric_limits<long double>::max(); return *this; }
    value /= o.value; return *this;
}
LongDouble LongDouble::Parse(const std::string& _s)
{
    try   { return LongDouble((long double)std::stold(_s)); }
    catch (const std::exception& e) { LOG_WARNING("LongDouble::Parse: cannot parse \"" + _s + "\": " + e.what()); return LongDouble((long double)0); }
}
LongDouble LongDouble::Clamp(const LongDouble& v, const LongDouble& mn, const LongDouble& mx)
{
    if (mn.value > mx.value) LOG_WARNING("LongDouble::Clamp: min > max.");
    return LongDouble(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
LongDouble LongDouble::Sqrt(const LongDouble& v)
{
    if (v.value < (long double)0) { LOG_ERROR("LongDouble::Sqrt: negative input. Returning 0."); return LongDouble((long double)0); }
    return LongDouble((long double)std::sqrt(v.value));
}
