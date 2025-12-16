#pragma once
#include "System/SubclassOf.h"

namespace Krampus
{

    template <typename Type, typename BaseType, IS_BASE_OF(BaseType, Type)>
    Type* Cast(BaseType* _baseType)
    {
        if (!_baseType) return nullptr;
        return dynamic_cast<Type*>(_baseType);
    }

    template<typename T, typename K>
    bool InstanceOf(K _object)
    {
        return std::is_same<T, decltype(_object)>::value;
    }
}