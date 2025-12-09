#pragma once
#include "Macro.h"

namespace Krampus
{
	template <typename Type>
	class SubclassOf
	{
		Type* objectRef = nullptr;

	public:
		FORCEINLINE Type GetObject() const
		{
			return *objectRef;
		}
		FORCEINLINE Type* GetObjectPtr() const
		{
			return objectRef;
		}
	public:
		SubclassOf() = default;
		SubclassOf(const Type& _objectRef)
		{
			objectRef = new Type(_objectRef);
		}
		SubclassOf(const SubclassOf<Type>& _other)
		{
			objectRef = new Type(_other.GetObject());
		}
		SubclassOf(Type&& _value)
		{
			objectRef = new Type(move(_value));
		}
		~SubclassOf()
		{
			delete objectRef;
		}

	public:
		SubclassOf& operator = (const SubclassOf<Type>& _other)
		{
			if (this != &_other)
			{
				delete objectRef;
			}

			objectRef = new Type(_other.GetObject());
			return *this;
		}
		SubclassOf& operator = (SubclassOf<Type>&& _other)
		{
			if (this != &_other)
			{
				delete objectRef;
			}

			objectRef = new Type(move(_other.GetObject()));
			return *this;
		}
	};
}