#pragma once
#include <functional>
#include "KrampusFwd.h"
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cstdlib>
#include <string>

class Int : public Krampus::IPrintable
{
    int value = 0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    Int() = default;
    Int(const Int& _o) : value(_o.value) {}
    Int& operator=(const Int&) = default;

    // Handles ALL arithmetic primitives via one template constructor
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int(T _v) : value(static_cast<int>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const Int& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<int>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }

    void Add     (const Int& _v) { value += _v.value; }
    void Subtract(const Int& _v) { value -= _v.value; }
    void Multiply(const Int& _v) { value *= _v.value; }
    void Divide  (const Int& _v);
    void Modulo  (const Int& _v);
    void And     (const Int& _v) { value &= _v.value; }
    void Or      (const Int& _v) { value |= _v.value; }
    void Xor     (const Int& _v) { value ^= _v.value; }
    void Not     ()                { value  = ~value;   }
    void ShiftLeft (int _b) { value <<= _b; }
    void ShiftRight(int _b) { value >>= _b; }

    void Abs();
    void Negate();
    void Invert();
    static Int Abs(const Int& _v);
    // Template variants – accept any arithmetic type
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (Int(static_cast<int>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(Int(static_cast<int>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(Int(static_cast<int>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (Int(static_cast<int>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Modulo  (T _v) { Modulo  (Int(static_cast<int>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero    () const { return Bool(value == 0); }
    Bool IsPositive() const { return Bool(value > (int)0); }
    Bool IsNegative() const { return Bool(value < (int)0); }
    Bool IsEven    () const { return Bool((value % 2) == 0); }
    Bool IsOdd     () const { return !IsEven(); }
    Bool IsEqual         (const Int& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const Int& o) const { return Bool(value != o.value); }
    Bool IsGreater       (const Int& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const Int& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const Int& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const Int& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static Int MaxInt() { return Int(std::numeric_limits<int>::max()); }
    static Int MinInt() { return Int(std::numeric_limits<int>::min()); }
    static Int Parse(const std::string& _s);
    static Int Max  (const Int& a, const Int& b) { return a.value >= b.value ? a : b; }
    static Int Min  (const Int& a, const Int& b) { return a.value <= b.value ? a : b; }
    static Int Clamp(const Int& v, const Int& mn, const Int& mx);

    // ── Operators (member) ───────────────────────────────────────────────────
    Int  operator+ (const Int& o) const { return Int((int)(value + o.value)); }
    Int  operator- (const Int& o) const { return Int((int)(value - o.value)); }
    Int  operator- ()               const { return Int(-value); }  // unary minus
    Int  operator* (const Int& o) const { return Int((int)(value * o.value)); }
    Int  operator/ (const Int& o) const;
    Int  operator% (const Int& o) const;
    Int  operator& (const Int& o) const { return Int(value & o.value); }
    Int  operator| (const Int& o) const { return Int(value | o.value); }
    Int  operator^ (const Int& o) const { return Int(value ^ o.value); }
    Int  operator~ ()               const { return Int(~value); }
    Int  operator<<(int b)          const { return Int(value << b); }
    Int  operator>>(int b)          const { return Int(value >> b); }

    Int& operator+=(const Int& o)       { value += o.value; return *this; }
    Int& operator-=(const Int& o)       { value -= o.value; return *this; }
    Int& operator*=(const Int& o)       { value *= o.value; return *this; }
    Int& operator/=(const Int& o);
    Int& operator%=(const Int& o);
    Int& operator&=(const Int& o)       { value &= o.value; return *this; }
    Int& operator|=(const Int& o)       { value |= o.value; return *this; }
    Int& operator^=(const Int& o)       { value ^= o.value; return *this; }
    Int& operator<<=(int b)               { value <<= b;      return *this; }
    Int& operator>>=(int b)               { value >>= b;      return *this; }

    Bool operator==(const Int& o) const { return Bool(value == o.value); }
    Bool operator!=(const Int& o) const { return Bool(value != o.value); }
    Bool operator< (const Int& o) const { return Bool(value <  o.value); }
    Bool operator> (const Int& o) const { return Bool(value >  o.value); }
    Bool operator<=(const Int& o) const { return Bool(value <= o.value); }
    Bool operator>=(const Int& o) const { return Bool(value >= o.value); }

    Int& operator++()    { ++value; return *this; }
    Int operator++(int) { Int _t(*this); value++; return _t; }
    Int& operator--()    { --value; return *this; }
    Int operator--(int) { Int _t(*this); value--; return _t; }

    // ── Template overloads: T op Int  and  Int op T ──────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int operator+(T v) const { return *this + Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int operator-(T v) const { return *this - Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int operator*(T v) const { return *this * Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int operator/(T v) const { return *this / Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int operator%(T v) const { return *this % Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int operator&(T v) const { return *this & Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int operator|(T v) const { return *this | Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int operator^(T v) const { return *this ^ Int(static_cast<int>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int& operator+=(T v) { return *this += Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int& operator-=(T v) { return *this -= Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int& operator*=(T v) { return *this *= Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int& operator/=(T v) { return *this /= Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int& operator%=(T v) { return *this %= Int(static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int& operator&=(T v) { value &= static_cast<int>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int& operator|=(T v) { value |= static_cast<int>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Int& operator^=(T v) { value ^= static_cast<int>(v); return *this; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<int>(v)); }

    // Non-member friends: T op Int
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Int operator+(T lhs, const Int& rhs) { return Int(static_cast<int>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Int operator-(T lhs, const Int& rhs) { return Int(static_cast<int>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Int operator*(T lhs, const Int& rhs) { return Int(static_cast<int>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Int operator/(T lhs, const Int& rhs) { return Int(static_cast<int>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Int operator%(T lhs, const Int& rhs) { return Int(static_cast<int>(lhs)) % rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Int operator&(T lhs, const Int& rhs) { return Int(static_cast<int>(lhs)) & rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Int operator|(T lhs, const Int& rhs) { return Int(static_cast<int>(lhs)) | rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Int operator^(T lhs, const Int& rhs) { return Int(static_cast<int>(lhs)) ^ rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const Int& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const Int& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const Int& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const Int& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const Int& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const Int& rhs) { return rhs <= lhs; }

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
    UInt ToUInt() const;
    Long ToLong() const;
    ULong ToULong() const;
    LongLong ToLongLong() const;
    ULongLong ToULongLong() const;
    Float ToFloat() const;
    Double ToDouble() const;
    LongDouble ToLongDouble() const;

};

// ─── std::hash specialization ────────────────────────────────────────────────
// Allows Int to be used as key in std::unordered_map / std::unordered_set
namespace std
{
    template<>
    struct hash<Int>
    {
        std::size_t operator()(const Int& _v) const noexcept
        {
            return std::hash<int>{}(static_cast<int>(_v));
        }
    };
}
