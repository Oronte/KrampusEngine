#include "Bool.h"

Bool::Bool(bool _bool)
	: value(_bool)
{
}

Bool::Bool(int _int)
{
	value = _int != 0;
}

Bool::Bool(const Bool& _other)
{
	value = _other.value;
}

void Bool::SetValue(const Bool& _value)
{
	value = _value.value;
}

void Bool::SetTrue()
{
	value = true;
}

void Bool::SetFalse()
{
	value = false;
}

Bool Bool::IsTrue() const
{
	return value ? True() : False();
}

Bool Bool::IsFalse() const
{
	return !IsTrue();
}

Bool Bool::IsEqual(const Bool& _other) const
{
	return value == _other.value;
}

Bool Bool::IsDifferent(const Bool& _other) const
{
	return !IsEqual(_other);
}

Bool Bool::And(const Bool& _other) const
{
	return *this && _other;
}

Bool Bool::Or(const Bool& _other) const
{
	return *this || _other;
}

Bool Bool::Xor(const Bool& _other) const
{
	return *this ^ _other;
}

Bool Bool::Not() const
{
	return !*this;
}

Bool Bool::Nand(const Bool& _other) const
{
	return !And(_other);
}

Bool Bool::Nor(const Bool& _other) const
{
	return !Or(_other);
}

std::string Bool::ToString() const
{
	return value ? "True" : "False";
}

Bool Bool::True()
{
	return Bool(true);
}

Bool Bool::False()
{
	return Bool(false);
}

Bool Bool::Parse(const std::string& _str)
{
	std::string _toLowerStr;
	_toLowerStr.reserve(_str.size());

	for (const char& _char : _str)
		_toLowerStr += std::tolower(_char);

	if (_toLowerStr == "true") 
		return true;
	if (_toLowerStr != "false")
		//LOG_WARNING("Incorrect string to parse! Value is set to False.");
		return false;
	return false;
	// TODO LOG
}

Bool Bool::operator==(const Bool& _other) const
{
	return Bool(value == _other.value);
}

Bool Bool::operator!=(const Bool& _other) const
{
	return Bool(value != _other.value);
}

Bool Bool::operator&&(const Bool& _other) const
{
	return Bool(value && _other.value);
}

Bool Bool::operator||(const Bool& _other) const
{
	return Bool(value || _other.value);
}

Bool Bool::operator!() const
{
	return Bool(!value);
}

Bool Bool::operator^(const Bool& _other) const
{
	return Bool(value ^ _other.value);
}