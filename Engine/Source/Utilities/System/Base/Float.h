#pragma once
#include "Bool.h"

class Float : public Krampus::IPrintable
{
	float value = 0.0f;

public:
	Float() = default;
	Float(float _value);
	Float(int _value);
	Float(double _value);
	Float(const Float& _other);

	void SetValue(const Float& _value);

	void Increment();
	void Decrement();

	void Add(const Float& _value);
	void Subtract(const Float& _value);
	void Multiply(const Float& _value);
	void Divide(const Float& _value);

	void Abs();
	void Negate();
	void Invert();

	Bool IsZero() const;
	Bool IsPositive() const;
	Bool IsNegative() const;

	Bool IsEqual(const Float& _other) const;
	Bool IsDifferent(const Float& _other) const;

	Bool IsGreater(const Float& _other) const;
	Bool IsLower(const Float& _other) const;

	Bool IsGreaterOrEqual(const Float& _other) const;
	Bool IsLowerOrEqual(const Float& _other) const;

	static Float MaxFloat();
	static Float MinFloat();

	static Float Max(const Float& a, const Float& b);
	static Float Min(const Float& a, const Float& b);

	static Float Abs(const Float& v);
	static Float Clamp(const Float& value, const Float& min, const Float& max);

	virtual std::string ToString() const override;

	Float operator+(const Float& _other) const;
	Float operator-(const Float& _other) const;
	Float operator*(const Float& _other) const;
	Float operator/(const Float& _other) const;

	Float& operator+=(const Float& _other);
	Float& operator-=(const Float& _other);
	Float& operator*=(const Float& _other);
	Float& operator/=(const Float& _other);

	Bool operator==(const Float& _other) const;
	Bool operator!=(const Float& _other) const;
	Bool operator<(const Float& _other) const;
	Bool operator>(const Float& _other) const;
	Bool operator<=(const Float& _other) const;
	Bool operator>=(const Float& _other) const;

	Float& operator++()
	{
		++value;
		return *this;
	}
	Float operator++(int)
	{
		value++;
		return *this;
	}

	Float& operator--()
	{
		--value;
		return *this;
	}
	Float operator--(int)
	{
		value--;
		return *this;
	}

	operator int() const
	{
		return (int)value;
	}
	operator float() const
	{
		return value;
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