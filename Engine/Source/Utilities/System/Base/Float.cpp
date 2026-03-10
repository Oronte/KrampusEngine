#include "Float.h"

Float::Float(float _value)
{
	value = _value;
}

Float::Float(int _value)
{
	value = (float)_value;
}

Float::Float(double _value)
{
	value = (float)_value;
}

Float::Float(const Float& _other)
{
	value = _other.value;
}

void Float::SetValue(const Float& _value)
{
	value = _value.value;
}

void Float::Increment()
{
	value++;
}

void Float::Decrement()
{
	value--;
}

void Float::Add(const Float& _value)
{
	value += _value.value;
}

void Float::Subtract(const Float& _value)
{
	value -= _value.value;
}

void Float::Multiply(const Float& _value)
{
	value *= _value.value;
}

void Float::Divide(const Float& _value)
{
	if (_value.IsZero())
	{
		//LOG_ERROR("Cannot divide by Zero! Value is set to max float.");
		//TODO LOG
		value = std::numeric_limits<float>::max();
		return;
	}
	value /= _value.value;
}

void Float::Abs()
{
	value = std::abs(value);
}

void Float::Negate()
{
	value = -std::abs(value);
}

void Float::Invert()
{
	value *= -1.0f;
}

Bool Float::IsZero() const
{
	return Bool(value == 0.0f);
}

Bool Float::IsPositive() const
{
	return Bool(value > 0.0f);
}

Bool Float::IsNegative() const
{
	return Bool(value < 0.0f);
}

Bool Float::IsEqual(const Float& _other) const
{
	return Bool(value == _other.value);
}

Bool Float::IsDifferent(const Float& _other) const
{
	return Bool(value != _other.value);
}

Bool Float::IsGreater(const Float& _other) const
{
	return Bool(value > _other.value);
}

Bool Float::IsLower(const Float& _other) const
{
	return Bool(value < _other.value);
}

Bool Float::IsGreaterOrEqual(const Float& _other) const
{
	return Bool(value >= _other.value);
}

Bool Float::IsLowerOrEqual(const Float& _other) const
{
	return Bool(value <= _other.value);
}

Float Float::MaxFloat()
{
	return Float(std::numeric_limits<float>::max());
}

Float Float::MinFloat()
{
	return Float(std::numeric_limits<float>::min());
}

Float Float::Max(const Float& a, const Float& b)
{
	float _a = a.value;
	float _b = b.value;
	return _a > _b ? Float(_a) : Float(_b);
}

Float Float::Min(const Float& a, const Float& b)
{
	float _a = a.value;
	float _b = b.value;
	return _a < _b ? Float(_a) : Float(_b);
}

Float Float::Abs(const Float& v)
{
	return Float(std::abs(v.value));
}

Float Float::Clamp(const Float& value, const Float& min, const Float& max)
{
	return Float(value.value < min.value ? min.value : value.value > max.value ? max.value : value.value);
}

std::string Float::ToString() const
{
	return std::to_string(value);
}

Float Float::operator+(const Float& _other) const
{
	return Float(value + _other.value);
}

Float Float::operator-(const Float& _other) const
{
	return Float(value - _other.value);
}

Float Float::operator*(const Float& _other) const
{
	return Float(value * _other.value);
}

Float Float::operator/(const Float& _other) const
{
	if (_other.IsZero())
	{
		//LOG_ERROR("Cannot divide by Zero! Return value is set to max float.");
		//TODO LOG
		return MaxFloat();
	}
	return Float(value / _other.value);
}

Float& Float::operator+=(const Float& _other)
{
	value += _other.value;
	return *this;
}

Float& Float::operator-=(const Float& _other)
{
	value -= _other.value;
	return *this;
}

Float& Float::operator*=(const Float& _other)
{
	value *= _other.value;
	return *this;
}

Float& Float::operator/=(const Float& _other)
{
	if (_other.IsZero())
	{
		//LOG_ERROR("Cannot divide by Zero! Return value is set to max float.");
		// TODO LOG
		value = std::numeric_limits<float>::max();
		return *this;
	}
	value /= _other.value;
	return *this;
}

Bool Float::operator==(const Float& _other) const
{
	return Bool(value == _other.value);
}

Bool Float::operator!=(const Float& _other) const
{
	return Bool(value != _other.value);
}

Bool Float::operator<(const Float& _other) const
{
	return Bool(value < _other.value);
}

Bool Float::operator>(const Float& _other) const
{
	return Bool(value > _other.value);
}

Bool Float::operator<=(const Float& _other) const
{
	return Bool(value <= _other.value);
}

Bool Float::operator>=(const Float& _other) const
{
	return Bool(value >= _other.value);
}
