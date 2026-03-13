#pragma once
#include "System/SubclassOf.h"

namespace Krampus
{

    /// <summary>
	/// Casts a base type pointer to a derived type pointer safely.
    /// </summary>
    /// <param name="_baseType"> -> The object that will be cast </param>
    template <typename Type, typename BaseType, IS_BASE_OF(BaseType, Type)>
    Type* Cast(BaseType* _baseType)
    {
        if (!_baseType) return nullptr;
        return dynamic_cast<Type*>(_baseType);
    }

    /// <summary>
	/// Checks if an object is an instance of a specific type.
    /// </summary>
    /// <param name="_object"> -> The item that will be checked</param>
    template<typename T, typename K>
    Bool InstanceOf(K _object)
    {
        return Bool(std::is_same<T, decltype(_object)>::value);
    }
}