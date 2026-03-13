#pragma once
#include "CoreMinimal.h"

namespace Krampus
{

	class IGameObject : public IPrintable
	{
	protected:
		String			name		= "KrampusGameObject";
		Bool			isActive	= true;

	public:
		void GetName(const String& _name)
		{
			name = _name;
		}
		String GetName() const
		{
			return name;
		}
		inline virtual void SetActive(const Bool& _active)
		{
			isActive = _active;
		}
		inline bool IsActive() const
		{
			return isActive;
		}

		IGameObject() = default;
		virtual ~IGameObject() = default;

		virtual void Construct() = 0;
		virtual void Deconstruct() = 0;
		virtual void BeginPlay() = 0;
		virtual void Tick(const Float& _deltaTime) = 0;
		virtual void BeginDestroy() = 0;

	public:
		virtual std::string ToString() const override
		{
			return std::format("{} -> Core : isActive = {}", name.StdString(), isActive.ToString());
		}
	};


}
