#pragma once
#include "Core/GameObject.h"
#include "components/Component.h"
#include "Utilities/Math/Transform.h"

namespace Krampus
{

	class Level;

	class Actor : public IGameObject
	{
		std::vector<Actor*> children;

	protected:
		std::vector<std::unique_ptr<Component>> components;
		Level* level;

		Transform oldTransform;
	public:
		Transform transform;

		Engine* GetWorld() const;

		template<typename Type = Level, IS_BASE_OF(Level, Type)>
		inline Type* GetLevel()
		{
			if (InstanceOf<Type>(level))
			{
				return level;
			}

			return Cast<Type>(level);
		}

		inline void AddChild(Actor* _actor)
		{
			if (std::find(children.begin(), children.end(), _actor) == children.end())
			{
				children.push_back(_actor);
			}
		}
		inline void RemoveChild(Actor* _toRemove)
		{
			std::erase_if(children, [&](Actor* _actor)
				{
					return _actor == _toRemove;
				});
		}

		Actor(Level* _level);

	protected:
		template <typename Type, typename ...Args, IS_BASE_OF(Component, Type)>
		inline Type* CreateComponent(Args&&... _args)
		{
			if (GetComponent<Type>())
			{
				LOG_ERROR("There is already component of this type");
				return nullptr;
			}

			std::unique_ptr<Type> _component = std::make_unique<Type>(this, std::forward<Args>(_args)...);
			Type* _rawComponent = _component.get();
			components.push_back(std::move(_component));
			_rawComponent->Construct();
			return _rawComponent;
		}

		template <typename Type, IS_BASE_OF(Component, Type)>
		inline void RemoveComponent()
		{
			Type* _component = GetComponent<Type>();
			if (!_component)
			{
				LOG_WARNING("There is not a component of this type");
				return;
			}

			_component->Deconstruct();
			std::find_if(components, [&](const std::unique_ptr<Component>& _item)
				{
					return _item.get() == _component;
				});

			return _component;
		}


		void Construct() override;
		void Deconstruct() override;
		void BeginPlay() override;
		void Tick(const float& _deltaTime) override;
		void BeginDestroy() override;
		
	public:
		virtual void SetActive(const bool& _status) override;

		template <typename Type, typename ...Args, IS_BASE_OF(Component, Type)>
		inline Type* GetComponent()
		{
			for (const std::unique_ptr<Component>& _component : components)
				if (Type* _castedComponent = Cast<Type>(_component.get())) return _castedComponent;

			return nullptr;
		}
		virtual void Destroy();

		friend class ActorManager;
	};

}



