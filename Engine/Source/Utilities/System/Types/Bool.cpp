#include "Bool.h"
#include "Utilities/Debug/Logger.h"
#include <algorithm>
#include <cctype>

Bool::Bool(bool _bool)  : value(_bool) {}
Bool::Bool(int  _int)   : value(_int != 0) {}
Bool::Bool(const Bool& _other) : value(_other.value) {}

void Bool::SetValue(const Bool& _v) { value = _v.value; }
void Bool::SetTrue()  { value = true;  }
void Bool::SetFalse() { value = false; }

Bool Bool::IsTrue()  const { return value; }
Bool Bool::IsFalse() const { return !value; }

Bool Bool::IsEqual    (const Bool& o) const { return value == o.value; }
Bool Bool::IsDifferent(const Bool& o) const { return value != o.value; }

Bool Bool::And (const Bool& o) const { return value && o.value; }
Bool Bool::Or  (const Bool& o) const { return value || o.value; }
Bool Bool::Xor (const Bool& o) const { return value ^  o.value; }
Bool Bool::Not ()              const { return !value; }
Bool Bool::Nand(const Bool& o) const { return !And(o); }
Bool Bool::Nor (const Bool& o) const { return !Or(o); }

std::string Bool::ToString() const { return value ? "True" : "False"; }

Bool Bool::True()  { return Bool(true);  }
Bool Bool::False() { return Bool(false); }

Bool Bool::Parse(const std::string& _str)
{
    std::string lo;
    lo.reserve(_str.size());
    for (char c : _str) lo += (char)std::tolower((unsigned char)c);
    if (lo == "true")  return true;
    if (lo == "false") return false;
    LOG_WARNING("Bool::Parse: unrecognised string \"" + _str + "\". Returning False.");
    return false;
}

Bool Bool::operator==(const Bool& o) const { return Bool(value == o.value); }
Bool Bool::operator!=(const Bool& o) const { return Bool(value != o.value); }
Bool Bool::operator&&(const Bool& o) const { return Bool(value && o.value); }
Bool Bool::operator||(const Bool& o) const { return Bool(value || o.value); }
Bool Bool::operator! ()              const { return Bool(!value); }
Bool Bool::operator^ (const Bool& o) const { return Bool(value ^  o.value); }

Bool Bool::operator==(bool o) const { return Bool(value == o); }
Bool Bool::operator!=(bool o) const { return Bool(value != o); }
Bool Bool::operator&&(bool o) const { return Bool(value && o); }
Bool Bool::operator||(bool o) const { return Bool(value || o); }
Bool Bool::operator^ (bool o) const { return Bool(value ^  o); }
