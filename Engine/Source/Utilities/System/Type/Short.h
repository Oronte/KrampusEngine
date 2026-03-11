#pragma once
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cstdlib>
#include <string>
#include "Utilities/System/Printable.h"

class Short : public Krampus::IPrintable
{
    short value = 0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    Short() = default;
    Short(const Short& _o) : value(_o.value) {}

    // Handles ALL arithmetic primitives via one template constructor
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short(T _v) : value(static_cast<short>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const Short& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<short>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }

    void Add     (const Short& _v) { value += _v.value; }
    void Subtract(const Short& _v) { value -= _v.value; }
    void Multiply(const Short& _v) { value *= _v.value; }
    void Divide  (const Short& _v);
    void Modulo  (const Short& _v);
    void And     (const Short& _v) { value &= _v.value; }
    void Or      (const Short& _v) { value |= _v.value; }
    void Xor     (const Short& _v) { value ^= _v.value; }
    void Not     ()                { value  = ~value;   }
    void ShiftLeft (int _b) { value <<= _b; }
    void ShiftRight(int _b) { value >>= _b; }

    void Abs();
    void Negate();
    void Invert();
    static Short Abs(const Short& _v);
    // Template variants – accept any arithmetic type
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (Short(static_cast<short>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(Short(static_cast<short>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(Short(static_cast<short>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (Short(static_cast<short>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Modulo  (T _v) { Modulo  (Short(static_cast<short>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero    () const { return Bool(value == 0); }
    Bool IsPositive() const { return Bool(value > (short)0); }
    Bool IsNegative() const { return Bool(value < (short)0); }
    Bool IsEven    () const { return Bool((value % 2) == 0); }
    Bool IsOdd     () const { return !IsEven(); }
    Bool IsEqual         (const Short& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const Short& o) const { return Bool(value != o.value); }
    Bool IsGreater       (const Short& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const Short& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const Short& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const Short& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static Short MaxShort() { return Short(std::numeric_limits<short>::max()); }
    static Short MinShort() { return Short(std::numeric_limits<short>::min()); }
    static Short Parse(const std::string& _s);
    static Short Max  (const Short& a, const Short& b) { return a.value >= b.value ? a : b; }
    static Short Min  (const Short& a, const Short& b) { return a.value <= b.value ? a : b; }
    static Short Clamp(const Short& v, const Short& mn, const Short& mx);

    // ── Operators (member) ───────────────────────────────────────────────────
    Short  operator+ (const Short& o) const { return Short((short)(value + o.value)); }
    Short  operator- (const Short& o) const { return Short((short)(value - o.value)); }
    Short  operator* (const Short& o) const { return Short((short)(value * o.value)); }
    Short  operator/ (const Short& o) const;
    Short  operator% (const Short& o) const;
    Short  operator& (const Short& o) const { return Short(value & o.value); }
    Short  operator| (const Short& o) const { return Short(value | o.value); }
    Short  operator^ (const Short& o) const { return Short(value ^ o.value); }
    Short  operator~ ()               const { return Short(~value); }
    Short  operator<<(int b)          const { return Short(value << b); }
    Short  operator>>(int b)          const { return Short(value >> b); }

    Short& operator+=(const Short& o)       { value += o.value; return *this; }
    Short& operator-=(const Short& o)       { value -= o.value; return *this; }
    Short& operator*=(const Short& o)       { value *= o.value; return *this; }
    Short& operator/=(const Short& o);
    Short& operator%=(const Short& o);
    Short& operator&=(const Short& o)       { value &= o.value; return *this; }
    Short& operator|=(const Short& o)       { value |= o.value; return *this; }
    Short& operator^=(const Short& o)       { value ^= o.value; return *this; }
    Short& operator<<=(int b)               { value <<= b;      return *this; }
    Short& operator>>=(int b)               { value >>= b;      return *this; }

    Bool operator==(const Short& o) const { return Bool(value == o.value); }
    Bool operator!=(const Short& o) const { return Bool(value != o.value); }
    Bool operator< (const Short& o) const { return Bool(value <  o.value); }
    Bool operator> (const Short& o) const { return Bool(value >  o.value); }
    Bool operator<=(const Short& o) const { return Bool(value <= o.value); }
    Bool operator>=(const Short& o) const { return Bool(value >= o.value); }

    Short& operator++()    { ++value; return *this; }
    Short  operator++(int) { Short t(*this); value++; return t; }
    Short& operator--()    { --value; return *this; }
    Short  operator--(int) { Short t(*this); value--; return t; }

    // ── Template overloads: T op Short  and  Short op T ──────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short operator+(T v) const { return *this + Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short operator-(T v) const { return *this - Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short operator*(T v) const { return *this * Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short operator/(T v) const { return *this / Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short operator%(T v) const { return *this % Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short operator&(T v) const { return *this & Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short operator|(T v) const { return *this | Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short operator^(T v) const { return *this ^ Short(static_cast<short>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short& operator+=(T v) { return *this += Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short& operator-=(T v) { return *this -= Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short& operator*=(T v) { return *this *= Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short& operator/=(T v) { return *this /= Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short& operator%=(T v) { return *this %= Short(static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short& operator&=(T v) { value &= static_cast<short>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short& operator|=(T v) { value |= static_cast<short>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Short& operator^=(T v) { value ^= static_cast<short>(v); return *this; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<short>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<short>(v)); }

    // Non-member friends: T op Short
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Short operator+(T lhs, const Short& rhs) { return Short(static_cast<short>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Short operator-(T lhs, const Short& rhs) { return Short(static_cast<short>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Short operator*(T lhs, const Short& rhs) { return Short(static_cast<short>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Short operator/(T lhs, const Short& rhs) { return Short(static_cast<short>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Short operator%(T lhs, const Short& rhs) { return Short(static_cast<short>(lhs)) % rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Short operator&(T lhs, const Short& rhs) { return Short(static_cast<short>(lhs)) & rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Short operator|(T lhs, const Short& rhs) { return Short(static_cast<short>(lhs)) | rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Short operator^(T lhs, const Short& rhs) { return Short(static_cast<short>(lhs)) ^ rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const Short& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const Short& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const Short& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const Short& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const Short& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const Short& rhs) { return rhs <= lhs; }

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
