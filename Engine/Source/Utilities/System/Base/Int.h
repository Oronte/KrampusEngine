#pragma once
#include "Bool.h"

class Int : public Krampus::IPrintable
{
	int value = 0;

public:
	Int() = default;
	Int(int _value);
	Int(float _value);
	Int(double _value);
	Int(const Int& _other);

	void SetValue(const Int& _value);

	void Increment();
	void Decrement();

	void Add(const Int& _value);
	void Subtract(const Int& _value);
	void Multiply(const Int& _value);
	void Divide(const Int& _value);
	void Modulo(const Int& _value);

	void Abs();
	void Negate();
	void Invert();

	Bool IsZero() const;
	Bool IsPositive() const;
	Bool IsNegative() const;

	Bool IsEven() const;
	Bool IsOdd() const;

	Bool IsEqual(const Int& _other) const;
	Bool IsDifferent(const Int& _other) const;

	Bool IsGreater(const Int& _other) const;
	Bool IsLower(const Int& _other) const;

	Bool IsGreaterOrEqual(const Int& _other) const;
	Bool IsLowerOrEqual(const Int& _other) const;

	static Int MaxInt();
	static Int MinInt();

	static Int Parse(const std::string& _str);

	static Int Max(const Int& a, const Int& b);
	static Int Min(const Int& a, const Int& b);

	static Int Abs(const Int& v);
	static Int Clamp(const Int& value, const Int& min, const Int& max);

	virtual std::string ToString() const override;

	Int operator+(const Int& _other) const;
	Int operator-(const Int& _other) const;
	Int operator*(const Int& _other) const;
	Int operator/(const Int& _other) const;
	Int operator%(const Int& _other) const;

	Int& operator+=(const Int& _other);
	Int& operator-=(const Int& _other);
	Int& operator*=(const Int& _other);
	Int& operator/=(const Int& _other);

	Bool operator==(const Int& _other) const;
	Bool operator!=(const Int& _other) const;
	Bool operator<(const Int& _other) const;
	Bool operator>(const Int& _other) const;
	Bool operator<=(const Int& _other) const;
	Bool operator>=(const Int& _other) const;

	Int& operator++();    // ++a
	Int operator++(int)	  // a++
	{
		value++;
		return *this;
	}

	Int& operator--();
	Int operator--(int)
	{
		value--;
		return *this;
	}

	operator int() const
	{
		return value;
	}
	operator float() const
	{
		return (float)value;
	}
	operator double() const
	{
		return (double)value;
	}
	operator bool() const
	{
		return value == 0 ? false : true;
	}
	operator Bool() const
	{
		return Bool(value == 0 ? false : true);
	}
};

