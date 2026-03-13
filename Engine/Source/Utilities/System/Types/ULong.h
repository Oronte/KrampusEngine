#pragma once
#include <functional>
#include "KrampusFwd.h"
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cstdlib>
#include <string>

class ULong : public Krampus::IPrintable
{
    unsigned long value = 0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    ULong() = default;
    ULong(const ULong& _o) : value(_o.value) {}
    ULong& operator=(const ULong&) = default;

    // Handles ALL arithmetic primitives via one template constructor
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong(T _v) : value(static_cast<unsigned long>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const ULong& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<unsigned long>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }

    void Add     (const ULong& _v) { value += _v.value; }
    void Subtract(const ULong& _v) { value -= _v.value; }
    void Multiply(const ULong& _v) { value *= _v.value; }
    void Divide  (const ULong& _v);
    void Modulo  (const ULong& _v);
    void And     (const ULong& _v) { value &= _v.value; }
    void Or      (const ULong& _v) { value |= _v.value; }
    void Xor     (const ULong& _v) { value ^= _v.value; }
    void Not     ()                { value  = ~value;   }
    void ShiftLeft (int _b) { value <<= _b; }
    void ShiftRight(int _b) { value >>= _b; }

    // Template variants – accept any arithmetic type
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (ULong(static_cast<unsigned long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(ULong(static_cast<unsigned long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(ULong(static_cast<unsigned long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (ULong(static_cast<unsigned long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Modulo  (T _v) { Modulo  (ULong(static_cast<unsigned long>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero    () const { return Bool(value == 0); }
    Bool IsPositive() const { return Bool(value > (unsigned long)0); }
    Bool IsNegative() const { return Bool(false); } // unsigned: always false
    Bool IsEven    () const { return Bool((value % 2) == 0); }
    Bool IsOdd     () const { return !IsEven(); }
    Bool IsEqual         (const ULong& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const ULong& o) const { return Bool(value != o.value); }
    Bool IsGreater       (const ULong& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const ULong& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const ULong& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const ULong& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static ULong MaxULong() { return ULong(std::numeric_limits<unsigned long>::max()); }
    static ULong MinULong() { return ULong(std::numeric_limits<unsigned long>::min()); }
    static ULong Parse(const std::string& _s);
    static ULong Max  (const ULong& a, const ULong& b) { return a.value >= b.value ? a : b; }
    static ULong Min  (const ULong& a, const ULong& b) { return a.value <= b.value ? a : b; }
    static ULong Clamp(const ULong& v, const ULong& mn, const ULong& mx);

    // ── Operators (member) ───────────────────────────────────────────────────
    ULong  operator+ (const ULong& o) const { return ULong((unsigned long)(value + o.value)); }
    ULong  operator- (const ULong& o) const { return ULong((unsigned long)(value - o.value)); }
    ULong  operator* (const ULong& o) const { return ULong((unsigned long)(value * o.value)); }
    ULong  operator/ (const ULong& o) const;
    ULong  operator% (const ULong& o) const;
    ULong  operator& (const ULong& o) const { return ULong(value & o.value); }
    ULong  operator| (const ULong& o) const { return ULong(value | o.value); }
    ULong  operator^ (const ULong& o) const { return ULong(value ^ o.value); }
    ULong  operator~ ()               const { return ULong(~value); }
    ULong  operator<<(int b)          const { return ULong(value << b); }
    ULong  operator>>(int b)          const { return ULong(value >> b); }

    ULong& operator+=(const ULong& o)       { value += o.value; return *this; }
    ULong& operator-=(const ULong& o)       { value -= o.value; return *this; }
    ULong& operator*=(const ULong& o)       { value *= o.value; return *this; }
    ULong& operator/=(const ULong& o);
    ULong& operator%=(const ULong& o);
    ULong& operator&=(const ULong& o)       { value &= o.value; return *this; }
    ULong& operator|=(const ULong& o)       { value |= o.value; return *this; }
    ULong& operator^=(const ULong& o)       { value ^= o.value; return *this; }
    ULong& operator<<=(int b)               { value <<= b;      return *this; }
    ULong& operator>>=(int b)               { value >>= b;      return *this; }

    Bool operator==(const ULong& o) const { return Bool(value == o.value); }
    Bool operator!=(const ULong& o) const { return Bool(value != o.value); }
    Bool operator< (const ULong& o) const { return Bool(value <  o.value); }
    Bool operator> (const ULong& o) const { return Bool(value >  o.value); }
    Bool operator<=(const ULong& o) const { return Bool(value <= o.value); }
    Bool operator>=(const ULong& o) const { return Bool(value >= o.value); }

    ULong& operator++()    { ++value; return *this; }
    ULong operator++(int) { ULong _t(*this); value++; return _t; }
    ULong& operator--()    { --value; return *this; }
    ULong operator--(int) { ULong _t(*this); value--; return _t; }

    // ── Template overloads: T op ULong  and  ULong op T ──────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong operator+(T v) const { return *this + ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong operator-(T v) const { return *this - ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong operator*(T v) const { return *this * ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong operator/(T v) const { return *this / ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong operator%(T v) const { return *this % ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong operator&(T v) const { return *this & ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong operator|(T v) const { return *this | ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong operator^(T v) const { return *this ^ ULong(static_cast<unsigned long>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong& operator+=(T v) { return *this += ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong& operator-=(T v) { return *this -= ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong& operator*=(T v) { return *this *= ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong& operator/=(T v) { return *this /= ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong& operator%=(T v) { return *this %= ULong(static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong& operator&=(T v) { value &= static_cast<unsigned long>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong& operator|=(T v) { value |= static_cast<unsigned long>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULong& operator^=(T v) { value ^= static_cast<unsigned long>(v); return *this; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<unsigned long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<unsigned long>(v)); }

    // Non-member friends: T op ULong
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULong operator+(T lhs, const ULong& rhs) { return ULong(static_cast<unsigned long>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULong operator-(T lhs, const ULong& rhs) { return ULong(static_cast<unsigned long>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULong operator*(T lhs, const ULong& rhs) { return ULong(static_cast<unsigned long>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULong operator/(T lhs, const ULong& rhs) { return ULong(static_cast<unsigned long>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULong operator%(T lhs, const ULong& rhs) { return ULong(static_cast<unsigned long>(lhs)) % rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULong operator&(T lhs, const ULong& rhs) { return ULong(static_cast<unsigned long>(lhs)) & rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULong operator|(T lhs, const ULong& rhs) { return ULong(static_cast<unsigned long>(lhs)) | rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULong operator^(T lhs, const ULong& rhs) { return ULong(static_cast<unsigned long>(lhs)) ^ rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const ULong& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const ULong& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const ULong& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const ULong& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const ULong& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const ULong& rhs) { return rhs <= lhs; }

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
    LongLong ToLongLong() const;
    ULongLong ToULongLong() const;
    Float ToFloat() const;
    Double ToDouble() const;
    LongDouble ToLongDouble() const;

};

// ─── std::hash specialization ────────────────────────────────────────────────
// Allows ULong to be used as key in std::unordered_map / std::unordered_set
namespace std
{
    template<>
    struct hash<ULong>
    {
        std::size_t operator()(const ULong& _v) const noexcept
        {
            return std::hash<unsigned long>{}(static_cast<unsigned long>(_v));
        }
    };
}
