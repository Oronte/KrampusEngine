#pragma once
#include "SubclassOf.h"

namespace Krampus
{
    //unsigned int GetUniqueID()
    //{
    //    static unsigned long long _uniqueId = 0;
    //    return ++_uniqueId;
    //}

    //template <typename Type, typename ...Args>
    //Type* Spawn(Args&&... _args)
    //{
    //    return new Type(forward<Args>(_args)...);
    //}

    //template <typename Type>
    //Type* Spawn(const SubclassOf<Type>& _ref)
    //{
    //    return new Type(_ref.GetObject());
    //}


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