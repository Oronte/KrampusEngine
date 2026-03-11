#pragma once
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cmath>
#include <string>
#include "Utilities/System/Printable.h"

class LongDouble : public Krampus::IPrintable
{
    long double value = (long double)0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    LongDouble() = default;
    LongDouble(const LongDouble& _o) : value(_o.value) {}

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongDouble(T _v) : value(static_cast<long double>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const LongDouble& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<long double>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }
    void Add     (const LongDouble& _v) { value += _v.value; }
    void Subtract(const LongDouble& _v) { value -= _v.value; }
    void Multiply(const LongDouble& _v) { value *= _v.value; }
    void Divide  (const LongDouble& _v);
    void Abs()    { value = (long double)std::abs(value); }
    void Negate() { value = -(long double)std::abs(value); }
    void Invert() { value *= (long double)-1; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (LongDouble(static_cast<long double>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(LongDouble(static_cast<long double>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(LongDouble(static_cast<long double>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (LongDouble(static_cast<long double>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero      () const { return Bool(value == (long double)0); }
    Bool IsPositive  () const { return Bool(value >  (long double)0); }
    Bool IsNegative  () const { return Bool(value <  (long double)0); }
    Bool IsNaN       () const { return Bool(std::isnan(value)); }
    Bool IsInfinite  () const { return Bool(std::isinf(value)); }
    Bool IsFinite    () const { return Bool(std::isfinite(value)); }
    Bool IsEqual         (const LongDouble& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const LongDouble& o) const { return Bool(value != o.value); }
    Bool IsNearlyEqual   (const LongDouble& o, const LongDouble& eps) const { return Bool(std::abs(value - o.value) <= eps.value); }
    Bool IsGreater       (const LongDouble& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const LongDouble& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const LongDouble& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const LongDouble& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static LongDouble MaxLongDouble()  { return LongDouble(std::numeric_limits<long double>::max()); }
    static LongDouble MinLongDouble()  { return LongDouble(std::numeric_limits<long double>::lowest()); }
    static LongDouble Parse(const std::string& _s);
    static LongDouble Max  (const LongDouble& a, const LongDouble& b)  { return a.value >= b.value ? a : b; }
    static LongDouble Min  (const LongDouble& a, const LongDouble& b)  { return a.value <= b.value ? a : b; }
    static LongDouble Abs  (const LongDouble& v)                  { return LongDouble((long double)std::abs(v.value)); }
    static LongDouble Clamp(const LongDouble& v, const LongDouble& mn, const LongDouble& mx);
    static LongDouble Lerp (const LongDouble& a, const LongDouble& b, const LongDouble& t) { return LongDouble(a.value + (b.value - a.value) * t.value); }
    static LongDouble Floor(const LongDouble& v) { return LongDouble((long double)std::floor(v.value)); }
    static LongDouble Ceil (const LongDouble& v) { return LongDouble((long double)std::ceil (v.value)); }
    static LongDouble Round(const LongDouble& v) { return LongDouble((long double)std::round(v.value)); }
    static LongDouble Sqrt (const LongDouble& v);
    static LongDouble Pow  (const LongDouble& base, const LongDouble& exp) { return LongDouble((long double)std::pow(base.value, exp.value)); }

    // ── Operators (member) ───────────────────────────────────────────────────
    LongDouble  operator+ (const LongDouble& o) const { return LongDouble(value + o.value); }
    LongDouble  operator- (const LongDouble& o) const { return LongDouble(value - o.value); }
    LongDouble  operator* (const LongDouble& o) const { return LongDouble(value * o.value); }
    LongDouble  operator/ (const LongDouble& o) const;
    LongDouble& operator+=(const LongDouble& o)       { value += o.value; return *this; }
    LongDouble& operator-=(const LongDouble& o)       { value -= o.value; return *this; }
    LongDouble& operator*=(const LongDouble& o)       { value *= o.value; return *this; }
    LongDouble& operator/=(const LongDouble& o);

    Bool operator==(const LongDouble& o) const { return Bool(value == o.value); }
    Bool operator!=(const LongDouble& o) const { return Bool(value != o.value); }
    Bool operator< (const LongDouble& o) const { return Bool(value <  o.value); }
    Bool operator> (const LongDouble& o) const { return Bool(value >  o.value); }
    Bool operator<=(const LongDouble& o) const { return Bool(value <= o.value); }
    Bool operator>=(const LongDouble& o) const { return Bool(value >= o.value); }

    LongDouble& operator++()    { ++value; return *this; }
    LongDouble  operator++(int) { LongDouble t(*this); value++; return t; }
    LongDouble& operator--()    { --value; return *this; }
    LongDouble  operator--(int) { LongDouble t(*this); value--; return t; }

    // ── Template overloads ────────────────────────────────────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongDouble operator+(T v) const { return *this + LongDouble(static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongDouble operator-(T v) const { return *this - LongDouble(static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongDouble operator*(T v) const { return *this * LongDouble(static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongDouble operator/(T v) const { return *this / LongDouble(static_cast<long double>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongDouble& operator+=(T v) { return *this += LongDouble(static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongDouble& operator-=(T v) { return *this -= LongDouble(static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongDouble& operator*=(T v) { return *this *= LongDouble(static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongDouble& operator/=(T v) { return *this /= LongDouble(static_cast<long double>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<long double>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<long double>(v)); }

    // Non-member friends
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongDouble operator+(T lhs, const LongDouble& rhs) { return LongDouble(static_cast<long double>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongDouble operator-(T lhs, const LongDouble& rhs) { return LongDouble(static_cast<long double>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongDouble operator*(T lhs, const LongDouble& rhs) { return LongDouble(static_cast<long double>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongDouble operator/(T lhs, const LongDouble& rhs) { return LongDouble(static_cast<long double>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const LongDouble& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const LongDouble& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const LongDouble& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const LongDouble& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const LongDouble& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const LongDouble& rhs) { return rhs <= lhs; }

    // ── Cast operators ───────────────────────────────────────────────────────

    operator bool()               const { return value != (long double)0; }
    operator Bool()               const { return Bool(value != (long double)0); }
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
};
