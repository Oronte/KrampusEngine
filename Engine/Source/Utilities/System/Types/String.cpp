#include "String.h"
#include "Utilities/Debug/Logger.h"
#include <cctype>
#include <sstream>

String::String(const char*        _v) : value(_v ? _v : "") {}
String::String(const std::string& _v) : value(_v) {}
String::String(const String&      _o) : value(_o.value) {}
String::String(char   _c) : value(1, _c) {}
String::String(int    _i) : value(std::to_string(_i)) {}
String::String(float  _f) : value(std::to_string(_f)) {}
String::String(double _d) : value(std::to_string(_d)) {}
String::String(bool   _b) : value(_b ? "True" : "False") {}

void String::SetValue(const String& _v) { value = _v.value; }
void String::Clear() { value.clear(); }
void String::Append (const String& s) { value += s.value; }
void String::Prepend(const String& s) { value  = s.value + value; }

void String::Insert(int _pos, const String& _s)
{
    if (_pos < 0 || _pos > (int)value.size())
    { LOG_WARNING("String::Insert: index out of range."); return; }
    value.insert((size_t)_pos, _s.value);
}

void String::Erase(int _pos, int _count)
{
    if (_pos < 0 || _pos >= (int)value.size())
    { LOG_WARNING("String::Erase: index out of range."); return; }
    value.erase((size_t)_pos, (size_t)_count);
}

void String::Replace(const String& _from, const String& _to)
{
    if (_from.value.empty()) { LOG_WARNING("String::Replace: empty search string."); return; }
    size_t pos = 0;
    while ((pos = value.find(_from.value, pos)) != std::string::npos)
    { value.replace(pos, _from.value.size(), _to.value); pos += _to.value.size(); }
}

void String::ToUpper() { for (char& c : value) c = (char)std::toupper((unsigned char)c); }
void String::ToLower() { for (char& c : value) c = (char)std::tolower((unsigned char)c); }

void String::TrimLeft()  { value.erase(0, value.find_first_not_of(" \t\n\r\f\v")); }
void String::TrimRight() { auto p = value.find_last_not_of(" \t\n\r\f\v"); value = (p == std::string::npos) ? "" : value.substr(0, p+1); }
void String::Trim()      { TrimLeft(); TrimRight(); }

void String::Reverse() { std::reverse(value.begin(), value.end()); }

void String::Reserve(const Int& _size)
{
    value.reserve(_size);
}

String String::Substring(int _pos, int _count) const
{
    if (_pos < 0 || _pos > (int)value.size()) { LOG_WARNING("String::Substring: index out of range. Returning empty."); return String(""); }
    return String(_count < 0 ? value.substr((size_t)_pos) : value.substr((size_t)_pos, (size_t)_count));
}

Int String::IndexOf(const String& _s, int _from) const
{
    size_t p = value.find(_s.value, (size_t)_from);
    return Int(p == std::string::npos ? -1 : (int)p);
}
Int String::LastIndexOf(const String& _s) const
{
    size_t p = value.rfind(_s.value);
    return Int(p == std::string::npos ? -1 : (int)p);
}
Int String::Length() const { return Int((int)value.size()); }
Int String::Size()   const { return Int((int)value.size()); }

Bool String::IsEmpty   () const { return Bool(value.empty()); }
Bool String::Contains  (const String& s) const { return Bool(value.find(s.value) != std::string::npos); }
Bool String::StartsWith(const String& s) const { return Bool(value.rfind(s.value, 0) == 0); }
Bool String::EndsWith  (const String& s) const
{
    if (s.value.size() > value.size()) return Bool(false);
    return Bool(value.compare(value.size() - s.value.size(), s.value.size(), s.value) == 0);
}
Bool String::IsEqual    (const String& o) const { return Bool(value == o.value); }
Bool String::IsDifferent(const String& o) const { return Bool(value != o.value); }
Bool String::IsEqualCI  (const String& o) const
{
    if (value.size() != o.value.size()) return Bool(false);
    for (size_t i = 0; i < value.size(); ++i)
        if (std::tolower((unsigned char)value[i]) != std::tolower((unsigned char)o.value[i])) return Bool(false);
    return Bool(true);
}

char String::At(int _i) const
{
    if (_i < 0 || _i >= (int)value.size()) { LOG_ERROR("String::At: index out of range. Returning '\\0'."); return '\0'; }
    return value[(size_t)_i];
}
char String::operator[](int _i) const { return At(_i); }

std::string String::ToString() const { return value; }

String String::FromInt   (int    _v) { return String(std::to_string(_v)); }
String String::FromFloat (float  _v) { return String(std::to_string(_v)); }
String String::FromDouble(double _v) { return String(std::to_string(_v)); }
String String::FromBool  (bool   _v) { return String(_v ? "True" : "False"); }

String  String::operator+ (const String& o) const { return String(value + o.value); }
String& String::operator+=(const String& o)       { value += o.value; return *this; }

Bool String::operator==(const String& o) const { return Bool(value == o.value); }
Bool String::operator!=(const String& o) const { return Bool(value != o.value); }
Bool String::operator< (const String& o) const { return Bool(value <  o.value); }
Bool String::operator> (const String& o) const { return Bool(value >  o.value); }
Bool String::operator<=(const String& o) const { return Bool(value <= o.value); }
Bool String::operator>=(const String& o) const { return Bool(value >= o.value); }

String  String::operator+ (const std::string& o) const { return String(value + o); }
String& String::operator+=(const std::string& o) { value += o; return *this; }

Bool String::operator==(const std::string& o) const { return Bool(value == o); }
Bool String::operator!=(const std::string& o) const { return Bool(value != o); }
Bool String::operator< (const std::string& o) const { return Bool(value < o); }
Bool String::operator> (const std::string& o) const { return Bool(value > o); }
Bool String::operator<=(const std::string& o) const { return Bool(value <= o); }
Bool String::operator>=(const std::string& o) const { return Bool(value >= o); }
