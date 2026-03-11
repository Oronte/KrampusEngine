#include "Char.h"
#include "Utilities/Debug/Logger.h"

Char::Char(char _v) : value(_v) {}
Char::Char(int  _v) : value((char)_v) {}
Char::Char(const Char& _o) : value(_o.value) {}

void Char::SetValue(const Char& _v) { value = _v.value; }

Bool Char::IsAlpha    () const { return Bool(std::isalpha ((unsigned char)value) != 0); }
Bool Char::IsDigit    () const { return Bool(std::isdigit ((unsigned char)value) != 0); }
Bool Char::IsAlNum    () const { return Bool(std::isalnum ((unsigned char)value) != 0); }
Bool Char::IsSpace    () const { return Bool(std::isspace ((unsigned char)value) != 0); }
Bool Char::IsUpper    () const { return Bool(std::isupper ((unsigned char)value) != 0); }
Bool Char::IsLower    () const { return Bool(std::islower ((unsigned char)value) != 0); }
Bool Char::IsPunct    () const { return Bool(std::ispunct ((unsigned char)value) != 0); }
Bool Char::IsPrintable() const { return Bool(std::isprint ((unsigned char)value) != 0); }

void Char::ToUpper() { value = (char)std::toupper((unsigned char)value); }
void Char::ToLower() { value = (char)std::tolower((unsigned char)value); }

Bool Char::IsEqual    (const Char& o) const { return Bool(value == o.value); }
Bool Char::IsDifferent(const Char& o) const { return Bool(value != o.value); }
Bool Char::IsGreater  (const Char& o) const { return Bool(value >  o.value); }
Bool Char::IsLower2   (const Char& o) const { return Bool(value <  o.value); }

std::string Char::ToString() const { return std::string(1, value); }

Char Char::Parse(const std::string& _str)
{
    if (_str.empty()) { LOG_WARNING("Char::Parse: empty string. Returning '\\0'."); return Char('\0'); }
    if (_str.size() > 1) LOG_WARNING("Char::Parse: string longer than 1, only first char taken.");
    return Char(_str[0]);
}

Bool Char::operator==(const Char& o) const { return Bool(value == o.value); }
Bool Char::operator!=(const Char& o) const { return Bool(value != o.value); }
Bool Char::operator< (const Char& o) const { return Bool(value <  o.value); }
Bool Char::operator> (const Char& o) const { return Bool(value >  o.value); }
Bool Char::operator<=(const Char& o) const { return Bool(value <= o.value); }
Bool Char::operator>=(const Char& o) const { return Bool(value >= o.value); }
