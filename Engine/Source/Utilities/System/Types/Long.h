#pragma once
#include <functional>
#include "KrampusFwd.h"
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cstdlib>
#include <string>

class Long : public Krampus::IPrintable
{
    long value = 0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    Long() = default;
    Long(const Long& _o) : value(_o.value) {}
    Long& operator=(const Long&) = default;

    // Handles ALL arithmetic primitives via one template constructor
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long(T _v) : value(static_cast<long>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const Long& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<long>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }

    void Add     (const Long& _v) { value += _v.value; }
    void Subtract(const Long& _v) { value -= _v.value; }
    void Multiply(const Long& _v) { value *= _v.value; }
    void Divide  (const Long& _v);
    void Modulo  (const Long& _v);
    void And     (const Long& _v) { value &= _v.value; }
    void Or      (const Long& _v) { value |= _v.value; }
    void Xor     (const Long& _v) { value ^= _v.value; }
    void Not     ()                { value  = ~value;   }
    void ShiftLeft (int _b) { value <<= _b; }
    void ShiftRight(int _b) { value >>= _b; }

    void Abs();
    void Negate();
    void Invert();
    static Long Abs(const Long& _v);
    // Template variants – accept any arithmetic type
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (Long(static_cast<long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(Long(static_cast<long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(Long(static_cast<long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (Long(static_cast<long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Modulo  (T _v) { Modulo  (Long(static_cast<long>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero    () const { return Bool(value == 0); }
    Bool IsPositive() const { return Bool(value > (long)0); }
    Bool IsNegative() const { return Bool(value < (long)0); }
    Bool IsEven    () const { return Bool((value % 2) == 0); }
    Bool IsOdd     () const { return !IsEven(); }
    Bool IsEqual         (const Long& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const Long& o) const { return Bool(value != o.value); }
    Bool IsGreater       (const Long& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const Long& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const Long& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const Long& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static Long MaxLong() { return Long(std::numeric_limits<long>::max()); }
    static Long MinLong() { return Long(std::numeric_limits<long>::min()); }
    static Long Parse(const std::string& _s);
    static Long Max  (const Long& a, const Long& b) { return a.value >= b.value ? a : b; }
    static Long Min  (const Long& a, const Long& b) { return a.value <= b.value ? a : b; }
    static Long Clamp(const Long& v, const Long& mn, const Long& mx);

    // ── Operators (member) ───────────────────────────────────────────────────
    Long  operator+ (const Long& o) const { return Long((long)(value + o.value)); }
    Long  operator- (const Long& o) const { return Long((long)(value - o.value)); }
    Long  operator- ()               const { return Long(-value); }  // unary minus
    Long  operator* (const Long& o) const { return Long((long)(value * o.value)); }
    Long  operator/ (const Long& o) const;
    Long  operator% (const Long& o) const;
    Long  operator& (const Long& o) const { return Long(value & o.value); }
    Long  operator| (const Long& o) const { return Long(value | o.value); }
    Long  operator^ (const Long& o) const { return Long(value ^ o.value); }
    Long  operator~ ()               const { return Long(~value); }
    Long  operator<<(int b)          const { return Long(value << b); }
    Long  operator>>(int b)          const { return Long(value >> b); }

    Long& operator+=(const Long& o)       { value += o.value; return *this; }
    Long& operator-=(const Long& o)       { value -= o.value; return *this; }
    Long& operator*=(const Long& o)       { value *= o.value; return *this; }
    Long& operator/=(const Long& o);
    Long& operator%=(const Long& o);
    Long& operator&=(const Long& o)       { value &= o.value; return *this; }
    Long& operator|=(const Long& o)       { value |= o.value; return *this; }
    Long& operator^=(const Long& o)       { value ^= o.value; return *this; }
    Long& operator<<=(int b)               { value <<= b;      return *this; }
    Long& operator>>=(int b)               { value >>= b;      return *this; }

    Bool operator==(const Long& o) const { return Bool(value == o.value); }
    Bool operator!=(const Long& o) const { return Bool(value != o.value); }
    Bool operator< (const Long& o) const { return Bool(value <  o.value); }
    Bool operator> (const Long& o) const { return Bool(value >  o.value); }
    Bool operator<=(const Long& o) const { return Bool(value <= o.value); }
    Bool operator>=(const Long& o) const { return Bool(value >= o.value); }

    Long& operator++()    { ++value; return *this; }
    Long operator++(int) { Long _t(*this); value++; return _t; }
    Long& operator--()    { --value; return *this; }
    Long operator--(int) { Long _t(*this); value--; return _t; }

    // ── Template overloads: T op Long  and  Long op T ──────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long operator+(T v) const { return *this + Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long operator-(T v) const { return *this - Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long operator*(T v) const { return *this * Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long operator/(T v) const { return *this / Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long operator%(T v) const { return *this % Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long operator&(T v) const { return *this & Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long operator|(T v) const { return *this | Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long operator^(T v) const { return *this ^ Long(static_cast<long>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long& operator+=(T v) { return *this += Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long& operator-=(T v) { return *this -= Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long& operator*=(T v) { return *this *= Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long& operator/=(T v) { return *this /= Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long& operator%=(T v) { return *this %= Long(static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long& operator&=(T v) { value &= static_cast<long>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long& operator|=(T v) { value |= static_cast<long>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Long& operator^=(T v) { value ^= static_cast<long>(v); return *this; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<long>(v)); }

    // Non-member friends: T op Long
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Long operator+(T lhs, const Long& rhs) { return Long(static_cast<long>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Long operator-(T lhs, const Long& rhs) { return Long(static_cast<long>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Long operator*(T lhs, const Long& rhs) { return Long(static_cast<long>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Long operator/(T lhs, const Long& rhs) { return Long(static_cast<long>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Long operator%(T lhs, const Long& rhs) { return Long(static_cast<long>(lhs)) % rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Long operator&(T lhs, const Long& rhs) { return Long(static_cast<long>(lhs)) & rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Long operator|(T lhs, const Long& rhs) { return Long(static_cast<long>(lhs)) | rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Long operator^(T lhs, const Long& rhs) { return Long(static_cast<long>(lhs)) ^ rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const Long& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const Long& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const Long& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const Long& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const Long& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const Long& rhs) { return rhs <= lhs; }

    // ── Cast operators ───────────────────────────────────────────────────────

    operator bool()               const { return value != 0; }
    operator Bool()               const { return Bool(value != 0); }
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
    ULong ToULong() const;
    LongLong ToLongLong() const;
    ULongLong ToULongLong() const;
    Float ToFloat() const;
    Double ToDouble() const;
    LongDouble ToLongDouble() const;

};

// ─── std::hash specialization ────────────────────────────────────────────────
// Allows Long to be used as key in std::unordered_map / std::unordered_set
namespace std
{
    template<>
    struct hash<Long>
    {
        std::size_t operator()(const Long& _v) const noexcept
        {
            return std::hash<long>{}(static_cast<long>(_v));
        }
    };
}
