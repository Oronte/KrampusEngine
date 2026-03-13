#include "Double.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void Double::Divide(const Double& _v)
{
    if (_v.value == (double)0) { LOG_ERROR("Double::Divide: division by zero. Value set to max."); value = std::numeric_limits<double>::max(); return; }
    value /= _v.value;
}
Double Double::operator/(const Double& o) const
{
    if (o.value == (double)0) { LOG_ERROR("Double::operator/: division by zero. Returning max."); return MaxDouble(); }
    return Double(value / o.value);
}
Double& Double::operator/=(const Double& o)
{
    if (o.value == (double)0) { LOG_ERROR("Double::operator/=: division by zero. Value set to max."); value = std::numeric_limits<double>::max(); return *this; }
    value /= o.value; return *this;
}
Double Double::Parse(const std::string& _s)
{
    try   { return Double((double)std::stold(_s)); }
    catch (const std::exception& e) { LOG_WARNING("Double::Parse: cannot parse \"" + _s + "\": " + e.what()); return Double((double)0); }
}
Double Double::Clamp(const Double& v, const Double& mn, const Double& mx)
{
    if (mn.value > mx.value) LOG_WARNING("Double::Clamp: min > max.");
    return Double(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
Double Double::Sqrt(const Double& v)
{
    if (v.value < (double)0) { LOG_ERROR("Double::Sqrt: negative input. Returning 0."); return Double((double)0); }
    return Double((double)std::sqrt(v.value));
}
