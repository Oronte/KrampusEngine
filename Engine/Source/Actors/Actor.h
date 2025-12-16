#pragma once
#include "Core/Core.h"
#include "components/Component.h"
#include "Utilities/Math/Transform.h"

namespace Krampus
{

	class Level;

	class Actor : public Core
	{
		bool toDelete = false;

	protected:
		std::vector<std::unique_ptr<Component>> components;
		Level* level;

	public:
		Transform transform;

		template<typename Type = Level, IS_BASE_OF(Level, Type)>
		INLINE Type* GetLevel()
		{
			if (InstanceOf<Type>(level))
			{
				return level;
			}

			return Cast<Type>(level);
		}
		INLINE bool IsToDelete() const noexcept
		{
			return toDelete;
		}

		Actor(Level* _level);

	protected:
		template <typename Type, typename ...Args, IS_BASE_OF(Component, Type)>
		INLINE Type* CreateComponent(Args&&... _args)
		{
			if (GetComponent<Type>())
			{
				LOG(VerbosityType::Error, "There is already component of this type");
				return nullptr;
			}

			std::unique_ptr<Type> _component = std::make_unique<Type>(this, std::forward<Args>(_args)...);
			Type* _rawComponent = _component.get();
			components.push_back(std::move(_component));
			_rawComponent->Construct();
			return _rawComponent;
		}

		virtual void SetActive(const bool& _status) override;

		void Construct() override;
		void Deconstruct() override;
		void BeginPlay() override;
		void Tick(const float& _deltaTime) override;
		void BeginDestroy() override;
		
	public:
		template <typename Type, typename ...Args, IS_BASE_OF(Component, Type)>
		INLINE Type* GetComponent()
		{
			for (const std::unique_ptr<Component>& _component : components)
				if (Type* _castedComponent = Cast<Type>(_component.get())) return _castedComponent;

			return nullptr;
		}
		void Destroy();

		friend class ActorManager;
	};

}



