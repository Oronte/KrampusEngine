#pragma once
#include <functional>
#include "Utilities/System/Printable.h"

class Bool : public Krampus::IPrintable
{
    bool value = false;

public:
    Bool() = default;
    Bool(bool _bool);
    Bool(int _int);
    Bool(const Bool& _other);

    void SetValue(const Bool& _value);
    void SetTrue();
    void SetFalse();

    Bool IsTrue()  const;
    Bool IsFalse() const;

    Bool IsEqual    (const Bool& _other) const;
    Bool IsDifferent(const Bool& _other) const;

    Bool And (const Bool& _other) const;
    Bool Or  (const Bool& _other) const;
    Bool Xor (const Bool& _other) const;
    Bool Not ()                   const;
    Bool Nand(const Bool& _other) const;
    Bool Nor (const Bool& _other) const;

    virtual std::string ToString() const override;

    static Bool True ();
    static Bool False();
    static Bool Parse(const std::string& _str);

    Bool  operator== (const Bool& _other) const;
    Bool  operator!= (const Bool& _other) const;
    Bool  operator&& (const Bool& _other) const;
    Bool  operator|| (const Bool& _other) const;
    Bool  operator!  ()                   const;
    Bool  operator^  (const Bool& _other) const;
    Bool& operator&= (const Bool& _other) { value &= _other.value; return *this; }
    Bool& operator|= (const Bool& _other) { value |= _other.value; return *this; }
    Bool& operator^= (const Bool& _other) { value ^= _other.value; return *this; }

    Bool  operator== (bool _other) const;
    Bool  operator!= (bool _other) const;
    Bool  operator&& (bool _other) const;
    Bool  operator|| (bool _other) const;
    Bool  operator^  (bool _other) const;
    Bool& operator&= (bool _other) { value &= _other; return *this; }
    Bool& operator|= (bool _other) { value |= _other; return *this; }
    Bool& operator^= (bool _other) { value ^= _other; return *this; }

    operator bool()       const { return value; }
    operator int()        const { return value ? 1 : 0; }
    operator float()      const { return value ? 1.0f : 0.0f; }
    operator double()     const { return value ? 1.0 : 0.0; }
};

// ─── std::hash specialization ────────────────────────────────────────────────
// Allows Bool to be used as key in std::unordered_map / std::unordered_set
namespace std
{
    template<>
    struct hash<Bool>
    {
        std::size_t operator()(const Bool& _v) const noexcept
        {
            return std::hash<bool>{}(static_cast<bool>(_v));
        }
    };
}
