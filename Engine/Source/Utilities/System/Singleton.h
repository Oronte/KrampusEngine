#pragma once
#include "Core/CoreMinimal.h"

namespace Krampus
{

	/// <summary>
	/// Provides a way to create a class that can have only one instance.
	/// </summary>
	template <typename Type>
	class Singleton
	{
	public:
		// Get/create the unique instance of Type
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