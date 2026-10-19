#pragma once
#include "WidgetComponents/WidgetComponent.h"
namespace Krampus
{
	class Widget : public IGameObject, public ITransformable
	{
		std::vector<Widget*>						children;
		Transform									transform;

	public:
		Event<FVector2>								onMove;
		Event<FVector2>								onScale;
		Event<Angle>								onRotate;

	protected:
		std::vector<std::unique_ptr<WidgetComponent>> components;
		Level*										level;

	public:
		Engine* GetWorld() const;

		inline void AddChild(Widget* _actor)
		{
			if (std::find(children.begin(), children.end(), _actor) == children.end())
			{
				children.push_back(_actor);
			}
		}
		inline void RemoveChild(Widget* _toRemove)
		{
			std::erase_if(children, [&](Widget* _actor)
				{
					return _actor == _toRemove;
				});
		}
		Float GetDeltaTime() const;

		Widget(Level* _level);

	protected:
		template <typename Type, typename ...Args, IS_BASE_OF(WidgetComponent, Type)>
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

		template <typename Type, IS_BASE_OF(WidgetComponent, Type)>
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
				[&](const std::unique_ptr<WidgetComponent>& _item)
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

		template <typename Type, typename ...Args, IS_BASE_OF(WidgetComponent, Type)>
		inline Type* GetComponent()
		{
			for (const std::unique_ptr<WidgetComponent>& _component : components)
				if (Type* _castedComponent = Cast<Type>(_component.get())) return _castedComponent;

			return nullptr;
		}
		virtual void Destroy();

		friend class HUD;
		friend class WidgetComponent;

		virtual std::string ToString() const override;

		void SetPosition(const FVector2& _newPosition) override;
		FVector2 GetPosition() const override;
		void SetRotation(const Angle& _newRotation) override;
		Angle GetRotation() const override;
		void SetScale(const FVector2& _newScale) override;
		FVector2 GetScale() const override;
};
}



