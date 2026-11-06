#pragma once
#include <cstdint>

// Engine names for the built-in arithmetic types. Plain aliases: no overhead, no wrapper API.
using Bool       = bool;
using Char       = char;
using Byte       = std::uint8_t;
using Short      = std::int16_t;
using UShort     = std::uint16_t;
using Int        = int;
using UInt       = unsigned int;
using Long       = long;
using ULong      = unsigned long;
using LongLong   = long long;
using ULongLong  = unsigned long long;
using Float      = float;
using Double     = double;
using LongDouble = long double;
