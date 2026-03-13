// TypeConversions.cpp
// Defines all ToXxx() cross-type conversion methods.
// This single translation unit includes all numeric headers,
// so circular dependencies are not an issue.
#include "Byte.h"
#include "Short.h"
#include "UShort.h"
#include "Int.h"
#include "UInt.h"
#include "Long.h"
#include "ULong.h"
#include "LongLong.h"
#include "ULongLong.h"
#include "Float.h"
#include "Double.h"
#include "LongDouble.h"

Short Byte::ToShort() const { return Short(static_cast<short>(value)); }
UShort Byte::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
Int Byte::ToInt() const { return Int(static_cast<int>(value)); }
UInt Byte::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
Long Byte::ToLong() const { return Long(static_cast<long>(value)); }
ULong Byte::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
LongLong Byte::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
ULongLong Byte::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Float Byte::ToFloat() const { return Float(static_cast<float>(value)); }
Double Byte::ToDouble() const { return Double(static_cast<double>(value)); }
LongDouble Byte::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte Short::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
UShort Short::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
Int Short::ToInt() const { return Int(static_cast<int>(value)); }
UInt Short::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
Long Short::ToLong() const { return Long(static_cast<long>(value)); }
ULong Short::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
LongLong Short::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
ULongLong Short::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Float Short::ToFloat() const { return Float(static_cast<float>(value)); }
Double Short::ToDouble() const { return Double(static_cast<double>(value)); }
LongDouble Short::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte UShort::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
Short UShort::ToShort() const { return Short(static_cast<short>(value)); }
Int UShort::ToInt() const { return Int(static_cast<int>(value)); }
UInt UShort::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
Long UShort::ToLong() const { return Long(static_cast<long>(value)); }
ULong UShort::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
LongLong UShort::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
ULongLong UShort::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Float UShort::ToFloat() const { return Float(static_cast<float>(value)); }
Double UShort::ToDouble() const { return Double(static_cast<double>(value)); }
LongDouble UShort::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte Int::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
Short Int::ToShort() const { return Short(static_cast<short>(value)); }
UShort Int::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
UInt Int::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
Long Int::ToLong() const { return Long(static_cast<long>(value)); }
ULong Int::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
LongLong Int::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
ULongLong Int::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Float Int::ToFloat() const { return Float(static_cast<float>(value)); }
Double Int::ToDouble() const { return Double(static_cast<double>(value)); }
LongDouble Int::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte UInt::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
Short UInt::ToShort() const { return Short(static_cast<short>(value)); }
UShort UInt::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
Int UInt::ToInt() const { return Int(static_cast<int>(value)); }
Long UInt::ToLong() const { return Long(static_cast<long>(value)); }
ULong UInt::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
LongLong UInt::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
ULongLong UInt::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Float UInt::ToFloat() const { return Float(static_cast<float>(value)); }
Double UInt::ToDouble() const { return Double(static_cast<double>(value)); }
LongDouble UInt::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte Long::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
Short Long::ToShort() const { return Short(static_cast<short>(value)); }
UShort Long::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
Int Long::ToInt() const { return Int(static_cast<int>(value)); }
UInt Long::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
ULong Long::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
LongLong Long::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
ULongLong Long::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Float Long::ToFloat() const { return Float(static_cast<float>(value)); }
Double Long::ToDouble() const { return Double(static_cast<double>(value)); }
LongDouble Long::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte ULong::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
Short ULong::ToShort() const { return Short(static_cast<short>(value)); }
UShort ULong::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
Int ULong::ToInt() const { return Int(static_cast<int>(value)); }
UInt ULong::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
Long ULong::ToLong() const { return Long(static_cast<long>(value)); }
LongLong ULong::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
ULongLong ULong::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Float ULong::ToFloat() const { return Float(static_cast<float>(value)); }
Double ULong::ToDouble() const { return Double(static_cast<double>(value)); }
LongDouble ULong::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte LongLong::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
Short LongLong::ToShort() const { return Short(static_cast<short>(value)); }
UShort LongLong::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
Int LongLong::ToInt() const { return Int(static_cast<int>(value)); }
UInt LongLong::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
Long LongLong::ToLong() const { return Long(static_cast<long>(value)); }
ULong LongLong::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
ULongLong LongLong::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Float LongLong::ToFloat() const { return Float(static_cast<float>(value)); }
Double LongLong::ToDouble() const { return Double(static_cast<double>(value)); }
LongDouble LongLong::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte ULongLong::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
Short ULongLong::ToShort() const { return Short(static_cast<short>(value)); }
UShort ULongLong::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
Int ULongLong::ToInt() const { return Int(static_cast<int>(value)); }
UInt ULongLong::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
Long ULongLong::ToLong() const { return Long(static_cast<long>(value)); }
ULong ULongLong::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
LongLong ULongLong::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
Float ULongLong::ToFloat() const { return Float(static_cast<float>(value)); }
Double ULongLong::ToDouble() const { return Double(static_cast<double>(value)); }
LongDouble ULongLong::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte Float::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
Short Float::ToShort() const { return Short(static_cast<short>(value)); }
UShort Float::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
Int Float::ToInt() const { return Int(static_cast<int>(value)); }
UInt Float::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
Long Float::ToLong() const { return Long(static_cast<long>(value)); }
ULong Float::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
LongLong Float::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
ULongLong Float::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Double Float::ToDouble() const { return Double(static_cast<double>(value)); }
LongDouble Float::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte Double::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
Short Double::ToShort() const { return Short(static_cast<short>(value)); }
UShort Double::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
Int Double::ToInt() const { return Int(static_cast<int>(value)); }
UInt Double::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
Long Double::ToLong() const { return Long(static_cast<long>(value)); }
ULong Double::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
LongLong Double::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
ULongLong Double::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Float Double::ToFloat() const { return Float(static_cast<float>(value)); }
LongDouble Double::ToLongDouble() const { return LongDouble(static_cast<long double>(value)); }
Byte LongDouble::ToByte() const { return Byte(static_cast<unsigned char>(value)); }
Short LongDouble::ToShort() const { return Short(static_cast<short>(value)); }
UShort LongDouble::ToUShort() const { return UShort(static_cast<unsigned short>(value)); }
Int LongDouble::ToInt() const { return Int(static_cast<int>(value)); }
UInt LongDouble::ToUInt() const { return UInt(static_cast<unsigned int>(value)); }
Long LongDouble::ToLong() const { return Long(static_cast<long>(value)); }
ULong LongDouble::ToULong() const { return ULong(static_cast<unsigned long>(value)); }
LongLong LongDouble::ToLongLong() const { return LongLong(static_cast<long long>(value)); }
ULongLong LongDouble::ToULongLong() const { return ULongLong(static_cast<unsigned long long>(value)); }
Float LongDouble::ToFloat() const { return Float(static_cast<float>(value)); }
Double LongDouble::ToDouble() const { return Double(static_cast<double>(value)); }
