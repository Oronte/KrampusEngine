#include "Float.h"
#include "Utilities/Debug/Logger.h"
#include <stdexcept>

void Float::Divide(const Float& _v)
{
    if (_v.value == (float)0) { LOG_ERROR("Float::Divide: division by zero. Value set to max."); value = std::numeric_limits<float>::max(); return; }
    value /= _v.value;
}
Float Float::operator/(const Float& o) const
{
    if (o.value == (float)0) { LOG_ERROR("Float::operator/: division by zero. Returning max."); return MaxFloat(); }
    return Float(value / o.value);
}
Float& Float::operator/=(const Float& o)
{
    if (o.value == (float)0) { LOG_ERROR("Float::operator/=: division by zero. Value set to max."); value = std::numeric_limits<float>::max(); return *this; }
    value /= o.value; return *this;
}
Float Float::Parse(const std::string& _s)
{
    try   { return Float((float)std::stold(_s)); }
    catch (const std::exception& e) { LOG_WARNING("Float::Parse: cannot parse \"" + _s + "\": " + e.what()); return Float((float)0); }
}
Float Float::Clamp(const Float& v, const Float& mn, const Float& mx)
{
    if (mn.value > mx.value) LOG_WARNING("Float::Clamp: min > max.");
    return Float(v.value < mn.value ? mn.value : v.value > mx.value ? mx.value : v.value);
}
Float Float::Sqrt(const Float& v)
{
    if (v.value < (float)0) { LOG_ERROR("Float::Sqrt: negative input. Returning 0."); return Float((float)0); }
    return Float((float)std::sqrt(v.value));
}
