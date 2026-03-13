#pragma once
#include <functional>
#include "KrampusFwd.h"
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cstdlib>
#include <string>

class ULongLong : public Krampus::IPrintable
{
    unsigned long long value = 0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    ULongLong() = default;
    ULongLong(const ULongLong& _o) : value(_o.value) {}
    ULongLong& operator=(const ULongLong&) = default;

    // Handles ALL arithmetic primitives via one template constructor
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong(T _v) : value(static_cast<unsigned long long>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const ULongLong& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<unsigned long long>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }

    void Add     (const ULongLong& _v) { value += _v.value; }
    void Subtract(const ULongLong& _v) { value -= _v.value; }
    void Multiply(const ULongLong& _v) { value *= _v.value; }
    void Divide  (const ULongLong& _v);
    void Modulo  (const ULongLong& _v);
    void And     (const ULongLong& _v) { value &= _v.value; }
    void Or      (const ULongLong& _v) { value |= _v.value; }
    void Xor     (const ULongLong& _v) { value ^= _v.value; }
    void Not     ()                { value  = ~value;   }
    void ShiftLeft (int _b) { value <<= _b; }
    void ShiftRight(int _b) { value >>= _b; }

    // Template variants – accept any arithmetic type
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (ULongLong(static_cast<unsigned long long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(ULongLong(static_cast<unsigned long long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(ULongLong(static_cast<unsigned long long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (ULongLong(static_cast<unsigned long long>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Modulo  (T _v) { Modulo  (ULongLong(static_cast<unsigned long long>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero    () const { return Bool(value == 0); }
    Bool IsPositive() const { return Bool(value > (unsigned long long)0); }
    Bool IsNegative() const { return Bool(false); } // unsigned: always false
    Bool IsEven    () const { return Bool((value % 2) == 0); }
    Bool IsOdd     () const { return !IsEven(); }
    Bool IsEqual         (const ULongLong& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const ULongLong& o) const { return Bool(value != o.value); }
    Bool IsGreater       (const ULongLong& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const ULongLong& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const ULongLong& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const ULongLong& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static ULongLong MaxULongLong() { return ULongLong(std::numeric_limits<unsigned long long>::max()); }
    static ULongLong MinULongLong() { return ULongLong(std::numeric_limits<unsigned long long>::min()); }
    static ULongLong Parse(const std::string& _s);
    static ULongLong Max  (const ULongLong& a, const ULongLong& b) { return a.value >= b.value ? a : b; }
    static ULongLong Min  (const ULongLong& a, const ULongLong& b) { return a.value <= b.value ? a : b; }
    static ULongLong Clamp(const ULongLong& v, const ULongLong& mn, const ULongLong& mx);

    // ── Operators (member) ───────────────────────────────────────────────────
    ULongLong  operator+ (const ULongLong& o) const { return ULongLong((unsigned long long)(value + o.value)); }
    ULongLong  operator- (const ULongLong& o) const { return ULongLong((unsigned long long)(value - o.value)); }
    ULongLong  operator* (const ULongLong& o) const { return ULongLong((unsigned long long)(value * o.value)); }
    ULongLong  operator/ (const ULongLong& o) const;
    ULongLong  operator% (const ULongLong& o) const;
    ULongLong  operator& (const ULongLong& o) const { return ULongLong(value & o.value); }
    ULongLong  operator| (const ULongLong& o) const { return ULongLong(value | o.value); }
    ULongLong  operator^ (const ULongLong& o) const { return ULongLong(value ^ o.value); }
    ULongLong  operator~ ()               const { return ULongLong(~value); }
    ULongLong  operator<<(int b)          const { return ULongLong(value << b); }
    ULongLong  operator>>(int b)          const { return ULongLong(value >> b); }

    ULongLong& operator+=(const ULongLong& o)       { value += o.value; return *this; }
    ULongLong& operator-=(const ULongLong& o)       { value -= o.value; return *this; }
    ULongLong& operator*=(const ULongLong& o)       { value *= o.value; return *this; }
    ULongLong& operator/=(const ULongLong& o);
    ULongLong& operator%=(const ULongLong& o);
    ULongLong& operator&=(const ULongLong& o)       { value &= o.value; return *this; }
    ULongLong& operator|=(const ULongLong& o)       { value |= o.value; return *this; }
    ULongLong& operator^=(const ULongLong& o)       { value ^= o.value; return *this; }
    ULongLong& operator<<=(int b)               { value <<= b;      return *this; }
    ULongLong& operator>>=(int b)               { value >>= b;      return *this; }

    Bool operator==(const ULongLong& o) const { return Bool(value == o.value); }
    Bool operator!=(const ULongLong& o) const { return Bool(value != o.value); }
    Bool operator< (const ULongLong& o) const { return Bool(value <  o.value); }
    Bool operator> (const ULongLong& o) const { return Bool(value >  o.value); }
    Bool operator<=(const ULongLong& o) const { return Bool(value <= o.value); }
    Bool operator>=(const ULongLong& o) const { return Bool(value >= o.value); }

    ULongLong& operator++()    { ++value; return *this; }
    ULongLong operator++(int) { ULongLong _t(*this); value++; return _t; }
    ULongLong& operator--()    { --value; return *this; }
    ULongLong operator--(int) { ULongLong _t(*this); value--; return _t; }

    // ── Template overloads: T op ULongLong  and  ULongLong op T ──────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong operator+(T v) const { return *this + ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong operator-(T v) const { return *this - ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong operator*(T v) const { return *this * ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong operator/(T v) const { return *this / ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong operator%(T v) const { return *this % ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong operator&(T v) const { return *this & ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong operator|(T v) const { return *this | ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong operator^(T v) const { return *this ^ ULongLong(static_cast<unsigned long long>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong& operator+=(T v) { return *this += ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong& operator-=(T v) { return *this -= ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong& operator*=(T v) { return *this *= ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong& operator/=(T v) { return *this /= ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong& operator%=(T v) { return *this %= ULongLong(static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong& operator&=(T v) { value &= static_cast<unsigned long long>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong& operator|=(T v) { value |= static_cast<unsigned long long>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    ULongLong& operator^=(T v) { value ^= static_cast<unsigned long long>(v); return *this; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<unsigned long long>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<unsigned long long>(v)); }

    // Non-member friends: T op ULongLong
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULongLong operator+(T lhs, const ULongLong& rhs) { return ULongLong(static_cast<unsigned long long>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULongLong operator-(T lhs, const ULongLong& rhs) { return ULongLong(static_cast<unsigned long long>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULongLong operator*(T lhs, const ULongLong& rhs) { return ULongLong(static_cast<unsigned long long>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULongLong operator/(T lhs, const ULongLong& rhs) { return ULongLong(static_cast<unsigned long long>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULongLong operator%(T lhs, const ULongLong& rhs) { return ULongLong(static_cast<unsigned long long>(lhs)) % rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULongLong operator&(T lhs, const ULongLong& rhs) { return ULongLong(static_cast<unsigned long long>(lhs)) & rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULongLong operator|(T lhs, const ULongLong& rhs) { return ULongLong(static_cast<unsigned long long>(lhs)) | rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend ULongLong operator^(T lhs, const ULongLong& rhs) { return ULongLong(static_cast<unsigned long long>(lhs)) ^ rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const ULongLong& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const ULongLong& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const ULongLong& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const ULongLong& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const ULongLong& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const ULongLong& rhs) { return rhs <= lhs; }

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
    LongLong ToLongLong() const;
    Float ToFloat() const;
    Double ToDouble() const;
    LongDouble ToLongDouble() const;

};

// ─── std::hash specialization ────────────────────────────────────────────────
// Allows ULongLong to be used as key in std::unordered_map / std::unordered_set
namespace std
{
    template<>
    struct hash<ULongLong>
    {
        std::size_t operator()(const ULongLong& _v) const noexcept
        {
            return std::hash<unsigned long long>{}(static_cast<unsigned long long>(_v));
        }
    };
}
