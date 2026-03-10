#pragma once
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cstdlib>
#include <string>
#include "Utilities/System/Printable.h"

class UShort : public Krampus::IPrintable
{
    unsigned short value = 0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    UShort() = default;
    UShort(const UShort& _o) : value(_o.value) {}

    // Handles ALL arithmetic primitives via one template constructor
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort(T _v) : value(static_cast<unsigned short>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const UShort& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<unsigned short>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }

    void Add     (const UShort& _v) { value += _v.value; }
    void Subtract(const UShort& _v) { value -= _v.value; }
    void Multiply(const UShort& _v) { value *= _v.value; }
    void Divide  (const UShort& _v);
    void Modulo  (const UShort& _v);
    void And     (const UShort& _v) { value &= _v.value; }
    void Or      (const UShort& _v) { value |= _v.value; }
    void Xor     (const UShort& _v) { value ^= _v.value; }
    void Not     ()                { value  = ~value;   }
    void ShiftLeft (int _b) { value <<= _b; }
    void ShiftRight(int _b) { value >>= _b; }

    // Template variants – accept any arithmetic type
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (UShort(static_cast<unsigned short>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(UShort(static_cast<unsigned short>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(UShort(static_cast<unsigned short>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (UShort(static_cast<unsigned short>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Modulo  (T _v) { Modulo  (UShort(static_cast<unsigned short>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero    () const { return Bool(value == 0); }
    Bool IsPositive() const { return Bool(value > (unsigned short)0); }
    Bool IsNegative() const { return Bool(false); } // unsigned: always false
    Bool IsEven    () const { return Bool((value % 2) == 0); }
    Bool IsOdd     () const { return !IsEven(); }
    Bool IsEqual         (const UShort& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const UShort& o) const { return Bool(value != o.value); }
    Bool IsGreater       (const UShort& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const UShort& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const UShort& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const UShort& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static UShort MaxUShort() { return UShort(std::numeric_limits<unsigned short>::max()); }
    static UShort MinUShort() { return UShort(std::numeric_limits<unsigned short>::min()); }
    static UShort Parse(const std::string& _s);
    static UShort Max  (const UShort& a, const UShort& b) { return a.value >= b.value ? a : b; }
    static UShort Min  (const UShort& a, const UShort& b) { return a.value <= b.value ? a : b; }
    static UShort Clamp(const UShort& v, const UShort& mn, const UShort& mx);

    // ── Operators (member) ───────────────────────────────────────────────────
    UShort  operator+ (const UShort& o) const { return UShort((unsigned short)(value + o.value)); }
    UShort  operator- (const UShort& o) const { return UShort((unsigned short)(value - o.value)); }
    UShort  operator* (const UShort& o) const { return UShort((unsigned short)(value * o.value)); }
    UShort  operator/ (const UShort& o) const;
    UShort  operator% (const UShort& o) const;
    UShort  operator& (const UShort& o) const { return UShort(value & o.value); }
    UShort  operator| (const UShort& o) const { return UShort(value | o.value); }
    UShort  operator^ (const UShort& o) const { return UShort(value ^ o.value); }
    UShort  operator~ ()               const { return UShort(~value); }
    UShort  operator<<(int b)          const { return UShort(value << b); }
    UShort  operator>>(int b)          const { return UShort(value >> b); }

    UShort& operator+=(const UShort& o)       { value += o.value; return *this; }
    UShort& operator-=(const UShort& o)       { value -= o.value; return *this; }
    UShort& operator*=(const UShort& o)       { value *= o.value; return *this; }
    UShort& operator/=(const UShort& o);
    UShort& operator%=(const UShort& o);
    UShort& operator&=(const UShort& o)       { value &= o.value; return *this; }
    UShort& operator|=(const UShort& o)       { value |= o.value; return *this; }
    UShort& operator^=(const UShort& o)       { value ^= o.value; return *this; }
    UShort& operator<<=(int b)               { value <<= b;      return *this; }
    UShort& operator>>=(int b)               { value >>= b;      return *this; }

    Bool operator==(const UShort& o) const { return Bool(value == o.value); }
    Bool operator!=(const UShort& o) const { return Bool(value != o.value); }
    Bool operator< (const UShort& o) const { return Bool(value <  o.value); }
    Bool operator> (const UShort& o) const { return Bool(value >  o.value); }
    Bool operator<=(const UShort& o) const { return Bool(value <= o.value); }
    Bool operator>=(const UShort& o) const { return Bool(value >= o.value); }

    UShort& operator++()    { ++value; return *this; }
    UShort  operator++(int) { UShort t(*this); value++; return t; }
    UShort& operator--()    { --value; return *this; }
    UShort  operator--(int) { UShort t(*this); value--; return t; }

    // ── Template overloads: T op UShort  and  UShort op T ──────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort operator+(T v) const { return *this + UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort operator-(T v) const { return *this - UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort operator*(T v) const { return *this * UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort operator/(T v) const { return *this / UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort operator%(T v) const { return *this % UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort operator&(T v) const { return *this & UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort operator|(T v) const { return *this | UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort operator^(T v) const { return *this ^ UShort(static_cast<unsigned short>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort& operator+=(T v) { return *this += UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort& operator-=(T v) { return *this -= UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort& operator*=(T v) { return *this *= UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort& operator/=(T v) { return *this /= UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort& operator%=(T v) { return *this %= UShort(static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort& operator&=(T v) { value &= static_cast<unsigned short>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort& operator|=(T v) { value |= static_cast<unsigned short>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    UShort& operator^=(T v) { value ^= static_cast<unsigned short>(v); return *this; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<unsigned short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<unsigned short>(v)); }

    // Non-member friends: T op UShort
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UShort operator+(T lhs, const UShort& rhs) { return UShort(static_cast<unsigned short>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UShort operator-(T lhs, const UShort& rhs) { return UShort(static_cast<unsigned short>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UShort operator*(T lhs, const UShort& rhs) { return UShort(static_cast<unsigned short>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UShort operator/(T lhs, const UShort& rhs) { return UShort(static_cast<unsigned short>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UShort operator%(T lhs, const UShort& rhs) { return UShort(static_cast<unsigned short>(lhs)) % rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UShort operator&(T lhs, const UShort& rhs) { return UShort(static_cast<unsigned short>(lhs)) & rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UShort operator|(T lhs, const UShort& rhs) { return UShort(static_cast<unsigned short>(lhs)) | rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend UShort operator^(T lhs, const UShort& rhs) { return UShort(static_cast<unsigned short>(lhs)) ^ rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const UShort& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const UShort& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const UShort& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const UShort& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const UShort& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const UShort& rhs) { return rhs <= lhs; }

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
