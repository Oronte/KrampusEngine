#include "Int.h"

Int::Int(int _value)
{
	value = _value;
}

Int::Int(float _value)
{
	value = (int)_value;
}

Int::Int(double _value)
{
	value = (int)_value;
}

Int::Int(const Int& _other)
{
	value = _other.value;
}

void Int::SetValue(const Int& _value)
{
	value = _value.value;
}

void Int::Increment()
{
	value++;
}

void Int::Decrement()
{
	value--;
}

void Int::Add(const Int& _value)
{
	value += _value.value;
}

void Int::Subtract(const Int& _value)
{
	value -= _value.value;
}

void Int::Multiply(const Int& _value)
{
	value *= _value.value;
}

void Int::Divide(const Int& _value)
{
	if (_value.IsZero())
	{
		//LOG_ERROR("Cannot divide by Zero! Value is set to max int.");
		//TODO LOG
		value = std::numeric_limits<int>::max();
		return;
	}
	value /= _value.value;
}

void Int::Modulo(const Int& _value)
{
	value %= _value.value;
}

void Int::Abs()
{
	value = std::abs(value);
}

void Int::Negate()
{
	value = -std::abs(value);
}

void Int::Invert()
{
	value *= -1;
}

Bool Int::IsZero() const
{
	return Bool(value == 0);
}

Bool Int::IsPositive() const
{
	return Bool(value > 0);
}

Bool Int::IsNegative() const
{
	return Bool(value < 0);
}

Bool Int::IsEven() const
{
	return Bool((value % 2) == 0);
}

Bool Int::IsOdd() const
{
	return !IsEven();
}

Bool Int::IsEqual(const Int& _other) const
{
	return Bool(value == _other.value);
}

Bool Int::IsDifferent(const Int& _other) const
{
	return !IsEqual(_other);
}

Bool Int::IsGreater(const Int& _other) const
{
	return Bool(value > _other.value);
}

Bool Int::IsLower(const Int& _other) const
{
	return Bool(value < _other.value);
}

Bool Int::IsGreaterOrEqual(const Int& _other) const
{
	return !IsLower(_other);
}

Bool Int::IsLowerOrEqual(const Int& _other) const
{
	return !IsGreater(_other);
}

Int Int::MaxInt()
{
	return Int(std::numeric_limits<int>::max());
}

Int Int::MinInt()
{
	return Int(std::numeric_limits<int>::min());
}

Int Int::Parse(const std::string& _str)
{
	return Int(std::stoi(_str));
}

Int Int::Max(const Int& _a, const Int& _b)
{
	int _aVal = _a.value;
	int _bVal = _b.value;
	return _aVal > _bVal ? Int(_aVal) : Int(_bVal);
}

Int Int::Min(const Int& _a, const Int& _b)
{
	int _aVal = _a.value;
	int _bVal = _b.value;
	return _aVal < _bVal ? Int(_aVal) : Int(_bVal);
}

Int Int::Abs(const Int& _value)
{
	return Int(std::abs(_value.value));
}

Int Int::Clamp(const Int& _value, const Int& _min, const Int& _max)
{
	return Int(_value.value < _min.value ? _min.value : _value.value > _max.value ? _max.value : _value.value);
}

std::string Int::ToString() const
{
	return std::to_string(value);
}

Int Int::operator+(const Int& _other) const
{
	return Int(_other.value + value);
}

Int Int::operator-(const Int& _other) const
{
	return Int(value - _other.value);
}

Int Int::operator*(const Int& _other) const
{
	return Int(value * _other.value);
}

Int Int::operator/(const Int& _other) const
{
	if (_other.IsZero())
	{
		//LOG_ERROR("Cannot divide by Zero! Return value is set to max int.");
		//TODO LOG
		return MaxInt();
	}
	return Int(value / _other.value);
}

Int Int::operator%(const Int& _other) const
{
	return Int(value % _other.value);
}

Int& Int::operator+=(const Int& _other)
{
	value += _other.value;
	return *this;
}

Int& Int::operator-=(const Int& _other)
{
	value -= _other.value;
	return *this;
}

Int& Int::operator*=(const Int& _other)
{
	value *= _other.value;
	return *this;
}

Int& Int::operator/=(const Int& _other)
{
	if (_other.IsZero())
	{
		//LOG_ERROR("Cannot divide by Zero! Value and the return value is set to max int.");
		//TODO LOG
		value = std::numeric_limits<int>::max();
		return *this;
	}
	value /= _other.value;
	return *this;
}

Bool Int::operator==(const Int& _other) const
{
	return Bool(value == _other.value);
}

Bool Int::operator!=(const Int& _other) const
{
	return Bool(value != _other.value);
}

Bool Int::operator<(const Int& _other) const
{
	return Bool(value < _other.value);
}

Bool Int::operator>(const Int& _other) const
{
	return Bool(value > _other.value);
}

Bool Int::operator<=(const Int& _other) const
{
	return Bool(value <= _other.value);
}

Bool Int::operator>=(const Int& _other) const
{
	return Bool(value >= _other.value);
}

Int& Int::operator++()
{
	++value;
	return *this;
}

Int& Int::operator--()
{
	--value;
	return *this;
}