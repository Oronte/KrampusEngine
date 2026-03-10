#pragma once
#include "Bool.h"
#include "Int.h"
#include <string>
#include <algorithm>
#include <stdexcept>
#include "Utilities/System/Printable.h"

class String : public Krampus::IPrintable
{
    std::string value;

public:
    String() = default;
    String(const char*        _value);
    String(const std::string& _value);
    String(const String&      _other);
    explicit String(char _c);
    explicit String(int  _i);
    explicit String(float _f);
    explicit String(double _d);
    explicit String(bool _b);

    void SetValue(const String& _value);
    void Clear();
    void Append   (const String& _str);
    void Prepend  (const String& _str);
    void Insert   (int _pos, const String& _str);
    void Erase    (int _pos, int _count = 1);
    void Replace  (const String& _from, const String& _to);
    void ToUpper  ();
    void ToLower  ();
    void Trim     ();
    void TrimLeft ();
    void TrimRight();
    void Reverse  ();

    String  Substring(int _pos, int _count = -1) const;
    Int     IndexOf  (const String& _str, int _from = 0) const;
    Int     LastIndexOf(const String& _str) const;
    Int     Length   () const;
    Int     Size     () const;

    Bool IsEmpty      () const;
    Bool Contains     (const String& _str) const;
    Bool StartsWith   (const String& _str) const;
    Bool EndsWith     (const String& _str) const;
    Bool IsEqual      (const String& _other) const;
    Bool IsDifferent  (const String& _other) const;
    Bool IsEqualCI    (const String& _other) const; // case-insensitive

    char At(int _index) const;
    char operator[](int _index) const;

    virtual std::string ToString() const override;

    const std::string& StdString() const { return value; }

    static String FromInt   (int    _v);
    static String FromFloat (float  _v);
    static String FromDouble(double _v);
    static String FromBool  (bool   _v);

    String  operator+ (const String& o) const;
    String& operator+=(const String& o);

    Bool    operator==(const String& o) const;
    Bool    operator!=(const String& o) const;
    Bool    operator< (const String& o) const;
    Bool    operator> (const String& o) const;
    Bool    operator<=(const String& o) const;
    Bool    operator>=(const String& o) const;

    operator std::string() const { return value; }
    operator const char*() const { return value.c_str(); }
    operator bool()        const { return !value.empty(); }
    operator Bool()        const { return Bool(!value.empty()); }
};
