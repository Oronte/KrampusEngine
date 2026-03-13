#pragma once
#include <functional>
#include "KrampusFwd.h"
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cmath>
#include <string>

class Float : public Krampus::IPrintable
{
    float value = (float)0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    Float() = default;
    Float(const Float& _o) : value(_o.value) {}
    Float& operator=(const Float&) = default;

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Float(T _v) : value(static_cast<float>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const Float& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<float>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }
    void Add     (const Float& _v) { value += _v.value; }
    void Subtract(const Float& _v) { value -= _v.value; }
    void Multiply(const Float& _v) { value *= _v.value; }
    void Divide  (const Float& _v);
    void Abs()    { value = (float)std::abs(value); }
    void Negate() { value = -(float)std::abs(value); }
    void Invert() { value *= (float)-1; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (Float(static_cast<float>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(Float(static_cast<float>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(Float(static_cast<float>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (Float(static_cast<float>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero      () const { return Bool(value == (float)0); }
    Bool IsPositive  () const { return Bool(value >  (float)0); }
    Bool IsNegative  () const { return Bool(value <  (float)0); }
    Bool IsNaN       () const { return Bool(std::isnan(value)); }
    Bool IsInfinite  () const { return Bool(std::isinf(value)); }
    Bool IsFinite    () const { return Bool(std::isfinite(value)); }
    Bool IsEqual         (const Float& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const Float& o) const { return Bool(value != o.value); }
    Bool IsNearlyEqual   (const Float& o, const Float& eps) const { return Bool(std::abs(value - o.value) <= eps.value); }
    Bool IsGreater       (const Float& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const Float& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const Float& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const Float& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static Float MaxFloat()  { return Float(std::numeric_limits<float>::max()); }
    static Float MinFloat()  { return Float(std::numeric_limits<float>::lowest()); }
    static Float Parse(const std::string& _s);
    static Float Max  (const Float& a, const Float& b)  { return a.value >= b.value ? a : b; }
    static Float Min  (const Float& a, const Float& b)  { return a.value <= b.value ? a : b; }
    static Float Abs  (const Float& v)                  { return Float((float)std::abs(v.value)); }
    static Float Clamp(const Float& v, const Float& mn, const Float& mx);
    static Float Lerp (const Float& a, const Float& b, const Float& t) { return Float(a.value + (b.value - a.value) * t.value); }
    static Float Floor(const Float& v) { return Float((float)std::floor(v.value)); }
    static Float Ceil (const Float& v) { return Float((float)std::ceil (v.value)); }
    static Float Round(const Float& v) { return Float((float)std::round(v.value)); }
    static Float Sqrt (const Float& v);
    static Float Pow  (const Float& base, const Float& exp) { return Float((float)std::pow(base.value, exp.value)); }

    // ── Operators (member) ───────────────────────────────────────────────────
    Float  operator+ (const Float& o) const { return Float(value + o.value); }
    Float  operator- (const Float& o) const { return Float(value - o.value); }
    Float  operator- ()               const { return Float(-value); }  // unary minus
    Float  operator* (const Float& o) const { return Float(value * o.value); }
    Float  operator/ (const Float& o) const;
    Float& operator+=(const Float& o)       { value += o.value; return *this; }
    Float& operator-=(const Float& o)       { value -= o.value; return *this; }
    Float& operator*=(const Float& o)       { value *= o.value; return *this; }
    Float& operator/=(const Float& o);

    Bool operator==(const Float& o) const { return Bool(value == o.value); }
    Bool operator!=(const Float& o) const { return Bool(value != o.value); }
    Bool operator< (const Float& o) const { return Bool(value <  o.value); }
    Bool operator> (const Float& o) const { return Bool(value >  o.value); }
    Bool operator<=(const Float& o) const { return Bool(value <= o.value); }
    Bool operator>=(const Float& o) const { return Bool(value >= o.value); }

    Float& operator++()    { ++value; return *this; }
    Float operator++(int) { Float _t(*this); value++; return _t; }
    Float& operator--()    { --value; return *this; }
    Float operator--(int) { Float _t(*this); value--; return _t; }

    // ── Template overloads ────────────────────────────────────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Float operator+(T v) const { return *this + Float(static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Float operator-(T v) const { return *this - Float(static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Float operator*(T v) const { return *this * Float(static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Float operator/(T v) const { return *this / Float(static_cast<float>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Float& operator+=(T v) { return *this += Float(static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Float& operator-=(T v) { return *this -= Float(static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Float& operator*=(T v) { return *this *= Float(static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Float& operator/=(T v) { return *this /= Float(static_cast<float>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<float>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<float>(v)); }

    // Non-member friends
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Float operator+(T lhs, const Float& rhs) { return Float(static_cast<float>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Float operator-(T lhs, const Float& rhs) { return Float(static_cast<float>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Float operator*(T lhs, const Float& rhs) { return Float(static_cast<float>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Float operator/(T lhs, const Float& rhs) { return Float(static_cast<float>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const Float& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const Float& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const Float& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const Float& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const Float& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const Float& rhs) { return rhs <= lhs; }

    // ── Cast operators ───────────────────────────────────────────────────────

    operator bool()               const { return value != (float)0; }
    operator Bool()               const { return Bool(value != (float)0); }
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
    Double ToDouble() const;
    LongDouble ToLongDouble() const;

};

// ─── std::hash specialization ────────────────────────────────────────────────
// Allows Float to be used as key in std::unordered_map / std::unordered_set
namespace std
{
    template<>
    struct hash<Float>
    {
        std::size_t operator()(const Float& _v) const noexcept
        {
            return std::hash<float>{}(static_cast<float>(_v));
        }
    };
}
