#pragma once
#include "CoreMinimal.h"

namespace Krampus
{

	class KObject : public IPrintable
	{
	protected:
		bool isActive = true;

	public:
		inline virtual void SetActive(const bool& _active)
		{
			isActive = _active;
		}
		inline bool IsActive() const
		{
			return isActive;
		}

		KObject() = default;
		virtual ~KObject() = default;

		virtual void Construct() = 0;
		virtual void Deconstruct() = 0;
		virtual void BeginPlay() = 0;
		virtual void Tick(const float& _deltaTime) = 0;
		virtual void BeginDestroy() = 0;

	public:
		virtual std::string ToString() const override
		{
			return std::format("Core : isActive = {}", isActive);
		}
	};


}
