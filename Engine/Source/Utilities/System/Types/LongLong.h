#pragma once
#include <functional>
#include "KrampusFwd.h"
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cstdlib>
#include <string>

class LongLong : public Krampus::IPrintable
{
    long long value = 0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    LongLong() = default;
    LongLong(const LongLong& _o) : value(_o.value) {}
    LongLong& operator=(const LongLong&) = default;

    // Handles ALL arithmetic primitives via one template constructor
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong(T _v) : value(static_cast<long long>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const LongLong& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<long long>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }

    void Add     (const LongLong& _v) { value += _v.value; }
    void Subtract(const LongLong& _v) { value -= _v.value; }
    void Multiply(const LongLong& _v) { value *= _v.value; }
    void Divide  (const LongLong& _v);
    void Modulo  (const LongLong& _v);
    void And     (const LongLong& _v) { value &= _v.value; }
    void Or      (const LongLong& _v) { value |= _v.value; }
    void Xor     (const LongLong& _v) { value ^= _v.value; }
    void Not     ()                { value  = ~value;   }
    void ShiftLeft (int _b) { value <<= _b; }
    void ShiftRight(int _b) { value >>= _b; }

    void Abs();
    void Negate();
    void Invert();
    static LongLong Abs(const LongLong& _v);
    // Template variants – accept any arithmetic type
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (LongLong(static_cast<long long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(LongLong(static_cast<long long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(LongLong(static_cast<long long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (LongLong(static_cast<long long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Modulo  (T _v) { Modulo  (LongLong(static_cast<long long>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero    () const { return Bool(value == 0); }
    Bool IsPositive() const { return Bool(value > (long long)0); }
    Bool IsNegative() const { return Bool(value < (long long)0); }
    Bool IsEven    () const { return Bool((value % 2) == 0); }
    Bool IsOdd     () const { return !IsEven(); }
    Bool IsEqual         (const LongLong& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const LongLong& o) const { return Bool(value != o.value); }
    Bool IsGreater       (const LongLong& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const LongLong& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const LongLong& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const LongLong& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static LongLong MaxLongLong() { return LongLong(std::numeric_limits<long long>::max()); }
    static LongLong MinLongLong() { return LongLong(std::numeric_limits<long long>::min()); }
    static LongLong Parse(const std::string& _s);
    static LongLong Max  (const LongLong& a, const LongLong& b) { return a.value >= b.value ? a : b; }
    static LongLong Min  (const LongLong& a, const LongLong& b) { return a.value <= b.value ? a : b; }
    static LongLong Clamp(const LongLong& v, const LongLong& mn, const LongLong& mx);

    // ── Operators (member) ───────────────────────────────────────────────────
    LongLong  operator+ (const LongLong& o) const { return LongLong((long long)(value + o.value)); }
    LongLong  operator- (const LongLong& o) const { return LongLong((long long)(value - o.value)); }
    LongLong  operator- ()               const { return LongLong(-value); }  // unary minus
    LongLong  operator* (const LongLong& o) const { return LongLong((long long)(value * o.value)); }
    LongLong  operator/ (const LongLong& o) const;
    LongLong  operator% (const LongLong& o) const;
    LongLong  operator& (const LongLong& o) const { return LongLong(value & o.value); }
    LongLong  operator| (const LongLong& o) const { return LongLong(value | o.value); }
    LongLong  operator^ (const LongLong& o) const { return LongLong(value ^ o.value); }
    LongLong  operator~ ()               const { return LongLong(~value); }
    LongLong  operator<<(int b)          const { return LongLong(value << b); }
    LongLong  operator>>(int b)          const { return LongLong(value >> b); }

    LongLong& operator+=(const LongLong& o)       { value += o.value; return *this; }
    LongLong& operator-=(const LongLong& o)       { value -= o.value; return *this; }
    LongLong& operator*=(const LongLong& o)       { value *= o.value; return *this; }
    LongLong& operator/=(const LongLong& o);
    LongLong& operator%=(const LongLong& o);
    LongLong& operator&=(const LongLong& o)       { value &= o.value; return *this; }
    LongLong& operator|=(const LongLong& o)       { value |= o.value; return *this; }
    LongLong& operator^=(const LongLong& o)       { value ^= o.value; return *this; }
    LongLong& operator<<=(int b)               { value <<= b;      return *this; }
    LongLong& operator>>=(int b)               { value >>= b;      return *this; }

    Bool operator==(const LongLong& o) const { return Bool(value == o.value); }
    Bool operator!=(const LongLong& o) const { return Bool(value != o.value); }
    Bool operator< (const LongLong& o) const { return Bool(value <  o.value); }
    Bool operator> (const LongLong& o) const { return Bool(value >  o.value); }
    Bool operator<=(const LongLong& o) const { return Bool(value <= o.value); }
    Bool operator>=(const LongLong& o) const { return Bool(value >= o.value); }

    LongLong& operator++()    { ++value; return *this; }
    LongLong operator++(int) { LongLong _t(*this); value++; return _t; }
    LongLong& operator--()    { --value; return *this; }
    LongLong operator--(int) { LongLong _t(*this); value--; return _t; }

    // ── Template overloads: T op LongLong  and  LongLong op T ──────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong operator+(T v) const { return *this + LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong operator-(T v) const { return *this - LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong operator*(T v) const { return *this * LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong operator/(T v) const { return *this / LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong operator%(T v) const { return *this % LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong operator&(T v) const { return *this & LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong operator|(T v) const { return *this | LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong operator^(T v) const { return *this ^ LongLong(static_cast<long long>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong& operator+=(T v) { return *this += LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong& operator-=(T v) { return *this -= LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong& operator*=(T v) { return *this *= LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong& operator/=(T v) { return *this /= LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong& operator%=(T v) { return *this %= LongLong(static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong& operator&=(T v) { value &= static_cast<long long>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong& operator|=(T v) { value |= static_cast<long long>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    LongLong& operator^=(T v) { value ^= static_cast<long long>(v); return *this; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<long long>(v)); }

    // Non-member friends: T op LongLong
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongLong operator+(T lhs, const LongLong& rhs) { return LongLong(static_cast<long long>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongLong operator-(T lhs, const LongLong& rhs) { return LongLong(static_cast<long long>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongLong operator*(T lhs, const LongLong& rhs) { return LongLong(static_cast<long long>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongLong operator/(T lhs, const LongLong& rhs) { return LongLong(static_cast<long long>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongLong operator%(T lhs, const LongLong& rhs) { return LongLong(static_cast<long long>(lhs)) % rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongLong operator&(T lhs, const LongLong& rhs) { return LongLong(static_cast<long long>(lhs)) & rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongLong operator|(T lhs, const LongLong& rhs) { return LongLong(static_cast<long long>(lhs)) | rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend LongLong operator^(T lhs, const LongLong& rhs) { return LongLong(static_cast<long long>(lhs)) ^ rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const LongLong& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const LongLong& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const LongLong& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const LongLong& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const LongLong& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const LongLong& rhs) { return rhs <= lhs; }

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
    Long ToLong() const;
    ULong ToULong() const;
    ULongLong ToULongLong() const;
    Float ToFloat() const;
    Double ToDouble() const;
    LongDouble ToLongDouble() const;

};

// ─── std::hash specialization ────────────────────────────────────────────────
// Allows LongLong to be used as key in std::unordered_map / std::unordered_set
namespace std
{
    template<>
    struct hash<LongLong>
    {
        std::size_t operator()(const LongLong& _v) const noexcept
        {
            return std::hash<long long>{}(static_cast<long long>(_v));
        }
    };
}
