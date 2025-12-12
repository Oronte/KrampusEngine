#pragma once
#include "Core/CoreMinimal.h"

namespace Krampus
{

	template <typename Type>
	class Singleton
	{
	public:
		INLINE static Type& GetInstance()
		{
			static Type _instance;
			return _instance;
		}
	};

}


///////////////////////////////////////////////
// 
//	class MyClass : public engine::Singleton<MyClass>
//  {
//		
//	}
//	
//	MyClass _myClassInstance = MyClass::GetInstance();
//
///////////////////////////////////////////////