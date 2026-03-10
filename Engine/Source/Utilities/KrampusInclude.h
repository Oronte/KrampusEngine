#pragma once
#include "Core/CoreMinimal.h"

//#define DEBUG_MEMORY_LEAKS
#ifdef DEBUG_MEMORY_LEAKS
#define new							new(_NORMAL_BLOCK, __FILE__, __LINE__)// Memory leaks
#else
#define new							nullptr; static_assert(false, "You cannot use the new operator. This prevents accidentaly memory leaks"); new
#endif