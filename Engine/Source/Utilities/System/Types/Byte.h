#pragma once
#include <functional>
#include "KrampusFwd.h"
#include "Bool.h"
#include <type_traits>
#include <limits>
#include <cstdlib>
#include <string>

class Byte : public Krampus::IPrintable
{
    unsigned char value = 0;

public:
    // ── Constructors ─────────────────────────────────────────────────────────
    Byte() = default;
    Byte(const Byte& _o) : value(_o.value) {}
    Byte& operator=(const Byte&) = default;

    // Handles ALL arithmetic primitives via one template constructor
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte(T _v) : value(static_cast<unsigned char>(_v)) {}

    // ── Assignment ───────────────────────────────────────────────────────────
    void SetValue(const Byte& _v) { value = _v.value; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void SetValue(T _v) { value = static_cast<unsigned char>(_v); }

    // ── Mutating arithmetic ───────────────────────────────────────────────────
    void Increment() { ++value; }
    void Decrement() { --value; }

    void Add     (const Byte& _v) { value += _v.value; }
    void Subtract(const Byte& _v) { value -= _v.value; }
    void Multiply(const Byte& _v) { value *= _v.value; }
    void Divide  (const Byte& _v);
    void Modulo  (const Byte& _v);
    void And     (const Byte& _v) { value &= _v.value; }
    void Or      (const Byte& _v) { value |= _v.value; }
    void Xor     (const Byte& _v) { value ^= _v.value; }
    void Not     ()                { value  = ~value;   }
    void ShiftLeft (int _b) { value <<= _b; }
    void ShiftRight(int _b) { value >>= _b; }

    // Template variants – accept any arithmetic type
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Add     (T _v) { Add     (Byte(static_cast<unsigned char>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Subtract(T _v) { Subtract(Byte(static_cast<unsigned char>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Multiply(T _v) { Multiply(Byte(static_cast<unsigned char>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Divide  (T _v) { Divide  (Byte(static_cast<unsigned char>(_v))); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    void Modulo  (T _v) { Modulo  (Byte(static_cast<unsigned char>(_v))); }

    // ── Queries ──────────────────────────────────────────────────────────────
    Bool IsZero    () const { return Bool(value == 0); }
    Bool IsPositive() const { return Bool(value > (unsigned char)0); }
    Bool IsNegative() const { return Bool(false); } // unsigned: always false
    Bool IsEven    () const { return Bool((value % 2) == 0); }
    Bool IsOdd     () const { return !IsEven(); }
    Bool IsEqual         (const Byte& o) const { return Bool(value == o.value); }
    Bool IsDifferent     (const Byte& o) const { return Bool(value != o.value); }
    Bool IsGreater       (const Byte& o) const { return Bool(value >  o.value); }
    Bool IsLower         (const Byte& o) const { return Bool(value <  o.value); }
    Bool IsGreaterOrEqual(const Byte& o) const { return Bool(value >= o.value); }
    Bool IsLowerOrEqual  (const Byte& o) const { return Bool(value <= o.value); }

    // ── Static helpers ───────────────────────────────────────────────────────
    virtual std::string ToString() const override { return std::to_string(value); }
    static Byte MaxByte() { return Byte(std::numeric_limits<unsigned char>::max()); }
    static Byte MinByte() { return Byte(std::numeric_limits<unsigned char>::min()); }
    static Byte Parse(const std::string& _s);
    static Byte Max  (const Byte& a, const Byte& b) { return a.value >= b.value ? a : b; }
    static Byte Min  (const Byte& a, const Byte& b) { return a.value <= b.value ? a : b; }
    static Byte Clamp(const Byte& v, const Byte& mn, const Byte& mx);

    // ── Operators (member) ───────────────────────────────────────────────────
    Byte  operator+ (const Byte& o) const { return Byte((unsigned char)(value + o.value)); }
    Byte  operator- (const Byte& o) const { return Byte((unsigned char)(value - o.value)); }
    Byte  operator* (const Byte& o) const { return Byte((unsigned char)(value * o.value)); }
    Byte  operator/ (const Byte& o) const;
    Byte  operator% (const Byte& o) const;
    Byte  operator& (const Byte& o) const { return Byte(value & o.value); }
    Byte  operator| (const Byte& o) const { return Byte(value | o.value); }
    Byte  operator^ (const Byte& o) const { return Byte(value ^ o.value); }
    Byte  operator~ ()               const { return Byte(~value); }
    Byte  operator<<(int b)          const { return Byte(value << b); }
    Byte  operator>>(int b)          const { return Byte(value >> b); }

    Byte& operator+=(const Byte& o)       { value += o.value; return *this; }
    Byte& operator-=(const Byte& o)       { value -= o.value; return *this; }
    Byte& operator*=(const Byte& o)       { value *= o.value; return *this; }
    Byte& operator/=(const Byte& o);
    Byte& operator%=(const Byte& o);
    Byte& operator&=(const Byte& o)       { value &= o.value; return *this; }
    Byte& operator|=(const Byte& o)       { value |= o.value; return *this; }
    Byte& operator^=(const Byte& o)       { value ^= o.value; return *this; }
    Byte& operator<<=(int b)               { value <<= b;      return *this; }
    Byte& operator>>=(int b)               { value >>= b;      return *this; }

    Bool operator==(const Byte& o) const { return Bool(value == o.value); }
    Bool operator!=(const Byte& o) const { return Bool(value != o.value); }
    Bool operator< (const Byte& o) const { return Bool(value <  o.value); }
    Bool operator> (const Byte& o) const { return Bool(value >  o.value); }
    Bool operator<=(const Byte& o) const { return Bool(value <= o.value); }
    Bool operator>=(const Byte& o) const { return Bool(value >= o.value); }

    Byte& operator++()    { ++value; return *this; }
    Byte operator++(int) { Byte _t(*this); value++; return _t; }
    Byte& operator--()    { --value; return *this; }
    Byte operator--(int) { Byte _t(*this); value--; return _t; }

    // ── Template overloads: T op Byte  and  Byte op T ──────────────────────
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte operator+(T v) const { return *this + Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte operator-(T v) const { return *this - Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte operator*(T v) const { return *this * Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte operator/(T v) const { return *this / Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte operator%(T v) const { return *this % Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte operator&(T v) const { return *this & Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte operator|(T v) const { return *this | Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte operator^(T v) const { return *this ^ Byte(static_cast<unsigned char>(v)); }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte& operator+=(T v) { return *this += Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte& operator-=(T v) { return *this -= Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte& operator*=(T v) { return *this *= Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte& operator/=(T v) { return *this /= Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte& operator%=(T v) { return *this %= Byte(static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte& operator&=(T v) { value &= static_cast<unsigned char>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte& operator|=(T v) { value |= static_cast<unsigned char>(v); return *this; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Byte& operator^=(T v) { value ^= static_cast<unsigned char>(v); return *this; }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator==(T v) const { return Bool(value == static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator!=(T v) const { return Bool(value != static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator< (T v) const { return Bool(value <  static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator> (T v) const { return Bool(value >  static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator<=(T v) const { return Bool(value <= static_cast<unsigned char>(v)); }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    Bool operator>=(T v) const { return Bool(value >= static_cast<unsigned char>(v)); }

    // Non-member friends: T op Byte
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Byte operator+(T lhs, const Byte& rhs) { return Byte(static_cast<unsigned char>(lhs)) + rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Byte operator-(T lhs, const Byte& rhs) { return Byte(static_cast<unsigned char>(lhs)) - rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Byte operator*(T lhs, const Byte& rhs) { return Byte(static_cast<unsigned char>(lhs)) * rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Byte operator/(T lhs, const Byte& rhs) { return Byte(static_cast<unsigned char>(lhs)) / rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Byte operator%(T lhs, const Byte& rhs) { return Byte(static_cast<unsigned char>(lhs)) % rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Byte operator&(T lhs, const Byte& rhs) { return Byte(static_cast<unsigned char>(lhs)) & rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Byte operator|(T lhs, const Byte& rhs) { return Byte(static_cast<unsigned char>(lhs)) | rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Byte operator^(T lhs, const Byte& rhs) { return Byte(static_cast<unsigned char>(lhs)) ^ rhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator==(T lhs, const Byte& rhs) { return rhs == lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator!=(T lhs, const Byte& rhs) { return rhs != lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator< (T lhs, const Byte& rhs) { return rhs >  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator> (T lhs, const Byte& rhs) { return rhs <  lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator<=(T lhs, const Byte& rhs) { return rhs >= lhs; }
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>> && !std::is_same_v<std::decay_t<T>, bool>>>
    friend Bool  operator>=(T lhs, const Byte& rhs) { return rhs <= lhs; }

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
    Short ToShort() const;
    UShort ToUShort() const;
    Int ToInt() const;
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
// Allows Byte to be used as key in std::unordered_map / std::unordered_set
namespace std
{
    template<>
    struct hash<Byte>
    {
        std::size_t operator()(const Byte& _v) const noexcept
        {
            return std::hash<unsigned char>{}(static_cast<unsigned char>(_v));
        }
    };
}
