#pragma once
#include <functional>
#include "KrampusFwd.h"
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cmath>
#include <string>

class Double : public Krampus::IPrintable
{
    double value = (double)0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    Double() = default;
    Double(const Double& _o) : value(_o.value) {}
    Double& operator=(const Double&) = default;

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Double(T _v) : value(static_cast<double>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const Double& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<double>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }
    void Add     (const Double& _v) { value += _v.value; }
    void Subtract(const Double& _v) { value -= _v.value; }
    void Multiply(const Double& _v) { value *= _v.value; }
    void Divide  (const Double& _v);
    void Abs()    { value = (double)std::abs(value); }
    void Negate() { value = -(double)std::abs(value); }
    void Invert() { value *= (double)-1; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (Double(static_cast<double>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(Double(static_cast<double>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(Double(static_cast<double>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (Double(static_cast<double>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero      () const { return Bool(value == (double)0); }
    Bool IsPositive  () const { return Bool(value >  (double)0); }
    Bool IsNegative  () const { return Bool(value <  (double)0); }
    Bool IsNaN       () const { return Bool(std::isnan(value)); }
    Bool IsInfinite  () const { return Bool(std::isinf(value)); }
    Bool IsFinite    () const { return Bool(std::isfinite(value)); }
    Bool IsEqual         (const Double& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const Double& o) const { return Bool(value != o.value); }
    Bool IsNearlyEqual   (const Double& o, const Double& eps) const { return Bool(std::abs(value - o.value) <= eps.value); }
    Bool IsGreater       (const Double& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const Double& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const Double& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const Double& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static Double MaxDouble()  { return Double(std::numeric_limits<double>::max()); }
    static Double MinDouble()  { return Double(std::numeric_limits<double>::lowest()); }
    static Double Parse(const std::string& _s);
    static Double Max  (const Double& a, const Double& b)  { return a.value >= b.value ? a : b; }
    static Double Min  (const Double& a, const Double& b)  { return a.value <= b.value ? a : b; }
    static Double Abs  (const Double& v)                  { return Double((double)std::abs(v.value)); }
    static Double Clamp(const Double& v, const Double& mn, const Double& mx);
    static Double Lerp (const Double& a, const Double& b, const Double& t) { return Double(a.value + (b.value - a.value) * t.value); }
    static Double Floor(const Double& v) { return Double((double)std::floor(v.value)); }
    static Double Ceil (const Double& v) { return Double((double)std::ceil (v.value)); }
    static Double Round(const Double& v) { return Double((double)std::round(v.value)); }
    static Double Sqrt (const Double& v);
    static Double Pow  (const Double& base, const Double& exp) { return Double((double)std::pow(base.value, exp.value)); }

    // ── Operators (member) ───────────────────────────────────────────────────
    Double  operator+ (const Double& o) const { return Double(value + o.value); }
    Double  operator- (const Double& o) const { return Double(value - o.value); }
    Double  operator- ()               const { return Double(-value); }  // unary minus
    Double  operator* (const Double& o) const { return Double(value * o.value); }
    Double  operator/ (const Double& o) const;
    Double& operator+=(const Double& o)       { value += o.value; return *this; }
    Double& operator-=(const Double& o)       { value -= o.value; return *this; }
    Double& operator*=(const Double& o)       { value *= o.value; return *this; }
    Double& operator/=(const Double& o);

    Bool operator==(const Double& o) const { return Bool(value == o.value); }
    Bool operator!=(const Double& o) const { return Bool(value != o.value); }
    Bool operator< (const Double& o) const { return Bool(value <  o.value); }
    Bool operator> (const Double& o) const { return Bool(value >  o.value); }
    Bool operator<=(const Double& o) const { return Bool(value <= o.value); }
    Bool operator>=(const Double& o) const { return Bool(value >= o.value); }

    Double& operator++()    { ++value; return *this; }
    Double operator++(int) { Double _t(*this); value++; return _t; }
    Double& operator--()    { --value; return *this; }
    Double operator--(int) { Double _t(*this); value--; return _t; }

    // ── Template overloads ────────────────────────────────────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Double operator+(T v) const { return *this + Double(static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Double operator-(T v) const { return *this - Double(static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Double operator*(T v) const { return *this * Double(static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Double operator/(T v) const { return *this / Double(static_cast<double>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Double& operator+=(T v) { return *this += Double(static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Double& operator-=(T v) { return *this -= Double(static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Double& operator*=(T v) { return *this *= Double(static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Double& operator/=(T v) { return *this /= Double(static_cast<double>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<double>(v)); }

    // Non-member friends
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Double operator+(T lhs, const Double& rhs) { return Double(static_cast<double>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Double operator-(T lhs, const Double& rhs) { return Double(static_cast<double>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Double operator*(T lhs, const Double& rhs) { return Double(static_cast<double>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Double operator/(T lhs, const Double& rhs) { return Double(static_cast<double>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const Double& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const Double& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const Double& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const Double& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const Double& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const Double& rhs) { return rhs <= lhs; }

    // ── Cast operators ───────────────────────────────────────────────────────

    operator bool()               const { return value != (double)0; }
    operator Bool()               const { return Bool(value != (double)0); }
    operator char()               const { return (char)(value); }
    operator unsigned char()      const { return (unsigned char)(value); }
    operator short()              const { return (short)(value); }
    operator unsigned short()     const { return (unsigned short)(value); }
    operator int()                const { return (int)(value); }
    operator unsigned int()       const { return (unsigned int)(value); }
    operator long()               const { return (long)(value); }
    operator unsigned long()      const { return (unsigned long)(value); }
    operator long long()          const { return (long long)(value); }
    operator unsigned long long() const { return (unsigned long long)(value); }
    operator float()              const { return (float)(value); }
    operator double()             const { return (double)(value); }
    operator long double()        const { return (long double)(value); }

    // ── Cross-type conversions ──────────────────────────────────────────────
    Byte ToByte() const;
    Short ToShort() const;
    UShort ToUShort() const;
    Int ToInt() const;
    UInt ToUInt() const;
    Long ToLong() const;
    ULong ToULong() const;
    LongLong ToLongLong() const;
    ULongLong ToULongLong() const;
    Float ToFloat() const;
    LongDouble ToLongDouble() const;

};

// ─── std::hash specialization ────────────────────────────────────────────────
// Allows Double to be used as key in std::unordered_map / std::unordered_set
namespace std
{
    template<>
    struct hash<Double>
    {
        std::size_t operator()(const Double& _v) const noexcept
        {
            return std::hash<double>{}(static_cast<double>(_v));
        }
    };
}
