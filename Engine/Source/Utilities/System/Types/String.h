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
    void Reserve(const Int& _size);

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
    const char* ToCharPtr() const { return value.c_str(); }

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

    String  operator+ (const std::string& o) const;
    String& operator+=(const std::string& o);

    Bool    operator==(const std::string& o) const;
    Bool    operator!=(const std::string& o) const;
    Bool    operator< (const std::string& o) const;
    Bool    operator> (const std::string& o) const;
    Bool    operator<=(const std::string& o) const;
    Bool    operator>=(const std::string& o) const;

    String  operator+ (const char* o) const { return String(value + (o ? o : "")); }
    String& operator+=(const char* o) { if (o) value += o; return *this; }
    Bool    operator==(const char* o) const { return Bool(o && value == o); }
    Bool    operator!=(const char* o) const { return Bool(!o || value != o); }
    Bool    operator< (const char* o) const { return Bool(o && value < o); }
    Bool    operator> (const char* o) const { return Bool(o && value > o); }
    Bool    operator<=(const char* o) const { return Bool(o && value <= o); }
    Bool    operator>=(const char* o) const { return Bool(o && value >= o); }

    String  operator+ (char c) const { return String(value + c); }
    String& operator+=(char c) { value += c; return *this; }
    Bool    operator==(char c) const { return Bool(value.size() == 1 && value[0] == c); }
    Bool    operator!=(char c) const { return Bool(value.size() != 1 || value[0] != c); }
    Bool    operator< (char c) const { return Bool(value < std::string(1, c)); }
    Bool    operator> (char c) const { return Bool(value > std::string(1, c)); }
    Bool    operator<=(char c) const { return Bool(value <= std::string(1, c)); }
    Bool    operator>=(char c) const { return Bool(value >= std::string(1, c)); }

    friend String operator+ (const std::string& lhs, const String& rhs)
    {
        return String(lhs + rhs.value);
    }
    friend Bool   operator==(const std::string& lhs, const String& rhs)
    {
        return Bool(lhs == rhs.value);
    }
    friend Bool   operator!=(const std::string& lhs, const String& rhs)
    {
        return Bool(lhs != rhs.value);
    }
    friend Bool   operator< (const std::string& lhs, const String& rhs)
    {
        return Bool(lhs < rhs.value);
    }
    friend Bool   operator> (const std::string& lhs, const String& rhs)
    {
        return Bool(lhs > rhs.value);
    }
    friend Bool   operator<=(const std::string& lhs, const String& rhs)
    {
        return Bool(lhs <= rhs.value);
    }
    friend Bool   operator>=(const std::string& lhs, const String& rhs)
    {
        return Bool(lhs >= rhs.value);
    }

    friend String operator+ (const char* lhs, const String& rhs)
    {
        return String((lhs ? lhs : "") + rhs.value);
    }
    friend Bool   operator==(const char* lhs, const String& rhs)
    {
        return Bool(lhs && rhs.value == lhs);
    }
    friend Bool   operator!=(const char* lhs, const String& rhs)
    {
        return Bool(!lhs || rhs.value != lhs);
    }
    friend Bool   operator< (const char* lhs, const String& rhs)
    {
        return Bool(lhs && std::string(lhs) < rhs.value);
    }
    friend Bool   operator> (const char* lhs, const String& rhs)
    {
        return Bool(lhs && std::string(lhs) > rhs.value);
    }
    friend Bool   operator<=(const char* lhs, const String& rhs)
    {
        return Bool(lhs && std::string(lhs) <= rhs.value);
    }
    friend Bool   operator>=(const char* lhs, const String& rhs)
    {
        return Bool(lhs && std::string(lhs) >= rhs.value);
    }

    friend String operator+ (char lhs, const String& rhs)
    {
        return String(std::string(1, lhs) + rhs.value);
    }
    friend Bool   operator==(char lhs, const String& rhs)
    {
        return Bool(rhs.value.size() == 1 && rhs.value[0] == lhs);
    }
    friend Bool   operator!=(char lhs, const String& rhs)
    {
        return Bool(rhs.value.size() != 1 || rhs.value[0] != lhs);
    }
    friend Bool   operator< (char lhs, const String& rhs)
    {
        return Bool(std::string(1, lhs) < rhs.value);
    }
    friend Bool   operator> (char lhs, const String& rhs)
    {
        return Bool(std::string(1, lhs) > rhs.value);
    }
    friend Bool   operator<=(char lhs, const String& rhs)
    {
        return Bool(std::string(1, lhs) <= rhs.value);
    }
    friend Bool   operator>=(char lhs, const String& rhs)
    {
        return Bool(std::string(1, lhs) >= rhs.value);
    }

    template<std::size_t N>
    String  operator+ (const char(&arr)[N]) const { return String(value + arr); }
    template<std::size_t N>
    String& operator+=(const char(&arr)[N]) { value += arr; return *this; }
    template<std::size_t N>
    Bool    operator==(const char(&arr)[N]) const { return Bool(value == arr); }
    template<std::size_t N>
    Bool    operator!=(const char(&arr)[N]) const { return Bool(value != arr); }
    template<std::size_t N>
    Bool    operator< (const char(&arr)[N]) const { return Bool(value < arr); }
    template<std::size_t N>
    Bool    operator> (const char(&arr)[N]) const { return Bool(value > arr); }
    template<std::size_t N>
    Bool    operator<=(const char(&arr)[N]) const { return Bool(value <= arr); }
    template<std::size_t N>
    Bool    operator>=(const char(&arr)[N]) const { return Bool(value >= arr); }

    template<std::size_t N>
    friend String operator+ (const char(&arr)[N], const String& rhs)
    {
        return String(std::string(arr) + rhs.value);
    }
    template<std::size_t N>
    friend Bool   operator==(const char(&arr)[N], const String& rhs)
    {
        return Bool(std::string(arr) == rhs.value);
    }
    template<std::size_t N>
    friend Bool   operator!=(const char(&arr)[N], const String& rhs)
    {
        return Bool(std::string(arr) != rhs.value);
    }
    template<std::size_t N>
    friend Bool   operator< (const char(&arr)[N], const String& rhs)
    {
        return Bool(std::string(arr) < rhs.value);
    }
    template<std::size_t N>
    friend Bool   operator> (const char(&arr)[N], const String& rhs)
    {
        return Bool(std::string(arr) > rhs.value);
    }
    template<std::size_t N>
    friend Bool   operator<=(const char(&arr)[N], const String& rhs)
    {
        return Bool(std::string(arr) <= rhs.value);
    }
    template<std::size_t N>
    friend Bool   operator>=(const char(&arr)[N], const String& rhs)
    {
        return Bool(std::string(arr) >= rhs.value);
    }

    operator std::string() const { return value; }
    operator const char*() const { return value.c_str(); }
    operator bool()        const { return !value.empty(); }
    operator Bool()        const { return Bool(!value.empty()); }
};


namespace std
{
    template<>
    struct hash<String>
    {
        size_t operator()(const String& s) const noexcept
        {
            return std::hash<std::string>()(s.ToString());
        }
    };
}