#pragma once
#include <functional>
#include "Bool.h"
#include <string>
#include <cctype>
#include <limits>

class Char : public Krampus::IPrintable
{
    char value = '\0';

public:
    Char() = default;
    explicit Char(char _value);
    Char(int _value);
    Char(const Char& _other);

    void SetValue(const Char& _value);

    Bool IsAlpha    () const;
    Bool IsDigit    () const;
    Bool IsAlNum    () const;
    Bool IsSpace    () const;
    Bool IsUpper    () const;
    Bool IsLower    () const;
    Bool IsPunct    () const;
    Bool IsPrintable() const;

    void ToUpper();
    void ToLower();

    Bool IsEqual    (const Char& _other) const;
    Bool IsDifferent(const Char& _other) const;
    Bool IsGreater  (const Char& _other) const;
    Bool IsLower2   (const Char& _other) const;

    virtual std::string ToString() const override;

    static Char Parse(const std::string& _str);

    Bool  operator==(const Char& o) const;
    Bool  operator!=(const Char& o) const;
    Bool  operator< (const Char& o) const;
    Bool  operator> (const Char& o) const;
    Bool  operator<=(const Char& o) const;
    Bool  operator>=(const Char& o) const;

    operator char()  const { return value; }
    operator int()   const { return (int)value; }
    operator bool()  const { return value != '\0'; }
    operator Bool()  const { return Bool(value != '\0'); }
};

// ─── std::hash specialization ────────────────────────────────────────────────
// Allows Char to be used as key in std::unordered_map / std::unordered_set
namespace std
{
    template<>
    struct hash<Char>
    {
        std::size_t operator()(const Char& _v) const noexcept
        {
            return std::hash<char>{}(static_cast<char>(_v));
        }
    };
}
