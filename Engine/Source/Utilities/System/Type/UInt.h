#pragma once
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cstdlib>
#include <string>
#include "Utilities/System/Printable.h"

class UInt : public Krampus::IPrintable
{
    unsigned int value = 0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    UInt() = default;
    UInt(const UInt& _o) : value(_o.value) {}

    // Handles ALL arithmetic primitives via one template constructor
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt(T _v) : value(static_cast<unsigned int>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const UInt& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<unsigned int>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }

    void Add     (const UInt& _v) { value += _v.value; }
    void Subtract(const UInt& _v) { value -= _v.value; }
    void Multiply(const UInt& _v) { value *= _v.value; }
    void Divide  (const UInt& _v);
    void Modulo  (const UInt& _v);
    void And     (const UInt& _v) { value &= _v.value; }
    void Or      (const UInt& _v) { value |= _v.value; }
    void Xor     (const UInt& _v) { value ^= _v.value; }
    void Not     ()                { value  = ~value;   }
    void ShiftLeft (int _b) { value <<= _b; }
    void ShiftRight(int _b) { value >>= _b; }

    // Template variants – accept any arithmetic type
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (UInt(static_cast<unsigned int>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(UInt(static_cast<unsigned int>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(UInt(static_cast<unsigned int>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (UInt(static_cast<unsigned int>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Modulo  (T _v) { Modulo  (UInt(static_cast<unsigned int>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero    () const { return Bool(value == 0); }
    Bool IsPositive() const { return Bool(value > (unsigned int)0); }
    Bool IsNegative() const { return Bool(false); } // unsigned: always false
    Bool IsEven    () const { return Bool((value % 2) == 0); }
    Bool IsOdd     () const { return !IsEven(); }
    Bool IsEqual         (const UInt& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const UInt& o) const { return Bool(value != o.value); }
    Bool IsGreater       (const UInt& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const UInt& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const UInt& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const UInt& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static UInt MaxUInt() { return UInt(std::numeric_limits<unsigned int>::max()); }
    static UInt MinUInt() { return UInt(std::numeric_limits<unsigned int>::min()); }
    static UInt Parse(const std::string& _s);
    static UInt Max  (const UInt& a, const UInt& b) { return a.value >= b.value ? a : b; }
    static UInt Min  (const UInt& a, const UInt& b) { return a.value <= b.value ? a : b; }
    static UInt Clamp(const UInt& v, const UInt& mn, const UInt& mx);

    // ── Operators (member) ───────────────────────────────────────────────────
    UInt  operator+ (const UInt& o) const { return UInt((unsigned int)(value + o.value)); }
    UInt  operator- (const UInt& o) const { return UInt((unsigned int)(value - o.value)); }
    UInt  operator* (const UInt& o) const { return UInt((unsigned int)(value * o.value)); }
    UInt  operator/ (const UInt& o) const;
    UInt  operator% (const UInt& o) const;
    UInt  operator& (const UInt& o) const { return UInt(value & o.value); }
    UInt  operator| (const UInt& o) const { return UInt(value | o.value); }
    UInt  operator^ (const UInt& o) const { return UInt(value ^ o.value); }
    UInt  operator~ ()               const { return UInt(~value); }
    UInt  operator<<(int b)          const { return UInt(value << b); }
    UInt  operator>>(int b)          const { return UInt(value >> b); }

    UInt& operator+=(const UInt& o)       { value += o.value; return *this; }
    UInt& operator-=(const UInt& o)       { value -= o.value; return *this; }
    UInt& operator*=(const UInt& o)       { value *= o.value; return *this; }
    UInt& operator/=(const UInt& o);
    UInt& operator%=(const UInt& o);
    UInt& operator&=(const UInt& o)       { value &= o.value; return *this; }
    UInt& operator|=(const UInt& o)       { value |= o.value; return *this; }
    UInt& operator^=(const UInt& o)       { value ^= o.value; return *this; }
    UInt& operator<<=(int b)               { value <<= b;      return *this; }
    UInt& operator>>=(int b)               { value >>= b;      return *this; }

    Bool operator==(const UInt& o) const { return Bool(value == o.value); }
    Bool operator!=(const UInt& o) const { return Bool(value != o.value); }
    Bool operator< (const UInt& o) const { return Bool(value <  o.value); }
    Bool operator> (const UInt& o) const { return Bool(value >  o.value); }
    Bool operator<=(const UInt& o) const { return Bool(value <= o.value); }
    Bool operator>=(const UInt& o) const { return Bool(value >= o.value); }

    UInt& operator++()    { ++value; return *this; }
    UInt  operator++(int) { UInt t(*this); value++; return t; }
    UInt& operator--()    { --value; return *this; }
    UInt  operator--(int) { UInt t(*this); value--; return t; }

    // ── Template overloads: T op UInt  and  UInt op T ──────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt operator+(T v) const { return *this + UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt operator-(T v) const { return *this - UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt operator*(T v) const { return *this * UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt operator/(T v) const { return *this / UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt operator%(T v) const { return *this % UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt operator&(T v) const { return *this & UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt operator|(T v) const { return *this | UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt operator^(T v) const { return *this ^ UInt(static_cast<unsigned int>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt& operator+=(T v) { return *this += UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt& operator-=(T v) { return *this -= UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt& operator*=(T v) { return *this *= UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt& operator/=(T v) { return *this /= UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt& operator%=(T v) { return *this %= UInt(static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt& operator&=(T v) { value &= static_cast<unsigned int>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt& operator|=(T v) { value |= static_cast<unsigned int>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UInt& operator^=(T v) { value ^= static_cast<unsigned int>(v); return *this; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<unsigned int>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<unsigned int>(v)); }

    // Non-member friends: T op UInt
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UInt operator+(T lhs, const UInt& rhs) { return UInt(static_cast<unsigned int>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UInt operator-(T lhs, const UInt& rhs) { return UInt(static_cast<unsigned int>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UInt operator*(T lhs, const UInt& rhs) { return UInt(static_cast<unsigned int>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UInt operator/(T lhs, const UInt& rhs) { return UInt(static_cast<unsigned int>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UInt operator%(T lhs, const UInt& rhs) { return UInt(static_cast<unsigned int>(lhs)) % rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UInt operator&(T lhs, const UInt& rhs) { return UInt(static_cast<unsigned int>(lhs)) & rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UInt operator|(T lhs, const UInt& rhs) { return UInt(static_cast<unsigned int>(lhs)) | rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UInt operator^(T lhs, const UInt& rhs) { return UInt(static_cast<unsigned int>(lhs)) ^ rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const UInt& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const UInt& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const UInt& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const UInt& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const UInt& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const UInt& rhs) { return rhs <= lhs; }

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
};
