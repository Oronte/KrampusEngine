#pragma once
#include "Utilities/System/Printable.h"
#include <string>

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
    Bool& operator&= (const Bool& _other) { value &= (bool)_other; return *this; }
    Bool& operator|= (const Bool& _other) { value |= (bool)_other; return *this; }
    Bool& operator^= (const Bool& _other) { value ^= (bool)_other; return *this; }

    operator bool()       const { return value; }
    operator int()        const { return value ? 1 : 0; }
    operator float()      const { return value ? 1.0f : 0.0f; }
    operator double()     const { return value ? 1.0 : 0.0; }
};
