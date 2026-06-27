#pragma once
#include "components/Component.h"

namespace Krampus
{

	class Level;

	class Actor : public IGameObject, public ITransformable
	{
		std::vector<Actor*>							children;
		Transform									transform;
	
	public:
		Event<FVector2>								onMove;
		Event<FVector2>								onScale;
		Event<Angle>								onRotate;

	protected:
		std::vector<std::unique_ptr<Component>>		components;
		Level*										level;

	public:
		Engine* GetWorld() const;

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
		Float GetDeltaTime() const;

	protected:
		Actor(Level* _level);

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
				LOG_WARNING("There is not a component of type" + std::string(NAME_OF(Type)));
				return;
			}

			_component->Deconstruct();
			_component->BeginDestroy();

			std::erase_if(components,
				[&](const std::unique_ptr<Component>& _item)
				{
					return _item.get() == _component;
				});
		}

		Timer* CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);


		void Construct() override;
		void Deconstruct() override;
		void BeginPlay() override;
		void Tick(const Float& _deltaTime) override;
		void BeginDestroy() override;
		
	public:
		template<typename Type = Level, IS_BASE_OF(Level, Type)>
		inline Type* GetLevel()
		{
			if (InstanceOf<Type>(level))
			{
				return level;
			}

			return Cast<Type>(level);
		}

		virtual void SetActive(const Bool& _status) override;

		template <typename Type, typename ...Args, IS_BASE_OF(Component, Type)>
		inline Type* GetComponent()
		{
			for (const std::unique_ptr<Component>& _component : components)
				if (Type* _castedComponent = Cast<Type>(_component.get())) return _castedComponent;

			return nullptr;
		}
		virtual void Destroy();

		friend class ActorManager;
		friend class Component;

		virtual std::string ToString() const override;

		void SetActorPosition(const FVector2& _newPosition) override;
		FVector2 GetActorPosition() const override;
		void SetActorRotation(const Angle& _newRotation) override;
		Angle GetActorRotation() const override;
		void SetActorScale(const FVector2& _newScale) override;
		FVector2 GetActorScale() const override;
};

}



