#pragma once
#include "WidgetComponents/WidgetComponent.h"

namespace Krampus
{
	/// <summary>
	/// Base class for all 2D screen-space heads-up display (HUD) widgets.
	/// </summary>
	class Widget : public IGameObject, public ITransformable
	{
		std::vector<Widget*>						children;
		Transform									transform;
		Bool										pendingDestroy	= false;

	public:
		/// <summary>Broadcast when widget position changes.</summary>
		Event<FVector2>								onMove;
		/// <summary>Broadcast when widget scale changes.</summary>
		Event<FVector2>								onScale;
		/// <summary>Broadcast when widget rotation changes.</summary>
		Event<Angle>								onRotate;

	protected:
		std::vector<std::unique_ptr<WidgetComponent>> components;
		Level*										level;

	public:
		/// <summary>Gets the engine world pointer.</summary>
		/// <returns>Pointer to Engine world.</returns>
		Engine* GetWorld() const;

		/// <summary>Adds a child widget to this widget hierarchy.</summary>
		/// <param name="_actor">Pointer to child Widget.</param>
		inline void AddChild(Widget* _actor)
		{
			if (std::find(children.begin(), children.end(), _actor) == children.end())
			{
				children.push_back(_actor);
			}
		}

		/// <summary>Gets the child widgets.</summary>
		inline const std::vector<Widget*>& GetChildren() const
		{
			return children;
		}

		/// <summary>Gets the components attached to this widget.</summary>
		inline const std::vector<std::unique_ptr<WidgetComponent>>& GetComponents() const
		{
			return components;
		}

		/// <summary>Public version of CreateComponent, for scenes and the editor.</summary>
		template <typename Type, typename ...Args, IS_BASE_OF(WidgetComponent, Type)>
		inline Type* AddComponent(Args&&... _args)
		{
			return CreateComponent<Type>(std::forward<Args>(_args)...);
		}

		/// <summary>Removes a component given by pointer (the editor does not know its type).</summary>
		inline void RemoveComponentInstance(WidgetComponent* _component)
		{
			if (!_component) return;
			LOG(VerbosityType::Verbose, "Component: " + std::string(typeid(*_component).name()) + " removed from " + GetName().StdString());
			_component->Deconstruct();
			_component->BeginDestroy();
			std::erase_if(components, [&](const std::unique_ptr<WidgetComponent>& _item) { return _item.get() == _component; });
		}

		/// <summary>Removes a child widget from hierarchy.</summary>
		/// <param name="_toRemove">Pointer to child Widget.</param>
		inline void RemoveChild(Widget* _toRemove)
		{
			std::erase_if(children, [&](Widget* _actor)
				{
					return _actor == _toRemove;
				});
		}

		/// <summary>Gets frame delta time in seconds.</summary>
		/// <returns>Delta time as Float.</returns>
		Float GetDeltaTime() const;

		/// <summary>Constructs a Widget associated with a level.</summary>
		/// <param name="_level">Pointer to owning Level.</param>
		Widget(Level* _level);

	protected:
		/// <summary>Creates and attaches a WidgetComponent to this widget.</summary>
		/// <typeparam name="Type">WidgetComponent derived type.</typeparam>
		/// <param name="_args">Arguments forwarded to constructor.</param>
		/// <returns>Pointer to newly created component.</returns>
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
			LOG(VerbosityType::Verbose, "Component: " + std::string(typeid(Type).name()) + " added to " + GetName().StdString());
			return _rawComponent;
		}

		/// <summary>Removes a component of the specified type from this widget.</summary>
		/// <typeparam name="Type">WidgetComponent type to remove.</typeparam>
		template <typename Type, IS_BASE_OF(WidgetComponent, Type)>
		inline void RemoveComponent()
		{
			Type* _component = GetComponent<Type>();
			if (!_component)
			{
				LOG_WARNING("There is not a component of type" + std::string(NAME_OF(Type)));
				return;
			}

			LOG(VerbosityType::Verbose, "Component: " + std::string(typeid(Type).name()) + " removed from " + GetName().StdString());
			_component->Deconstruct();
			_component->BeginDestroy();

			std::erase_if(components,
				[&](const std::unique_ptr<WidgetComponent>& _item)
				{
					return _item.get() == _component;
				});
		}

		/// <summary>Creates a timer managed within the widget's level.</summary>
		/// <param name="_callback">Timeout callback function.</param>
		/// <param name="_duration">Timer duration in seconds.</param>
		/// <param name="_isLoop">Whether timer repeats.</param>
		/// <param name="_startRunning">Whether timer starts immediately.</param>
		/// <returns>Pointer to created Timer.</returns>
		Timer* CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);

		/// <summary>Lifecycle callback on widget creation.</summary>
		void Construct() override;
		/// <summary>Lifecycle callback on widget deconstruction.</summary>
		void Deconstruct() override;
		/// <summary>Lifecycle callback when gameplay starts.</summary>
		void BeginPlay() override;
		/// <summary>Updates the widget each frame.</summary>
		/// <param name="_deltaTime">Elapsed time in seconds.</param>
		void Tick(const Float& _deltaTime) override;
		/// <summary>Lifecycle callback before widget is destroyed.</summary>
		void BeginDestroy() override;

	public:
		/// <summary>Gets the owning Level cast to requested type.</summary>
		/// <typeparam name="Type">Level derived type.</typeparam>
		/// <returns>Pointer to Level instance.</returns>
		template<typename Type = Level, IS_BASE_OF(Level, Type)>
		inline Type* GetLevel()
		{
			if constexpr (std::is_same_v<Type, Level>)
				return level;
			else
				return Cast<Type>(level);
		}

		/// <summary>True once Destroy() was called: the widget is waiting to be deleted at the end of the frame.</summary>
		inline Bool IsPendingDestroy() const
		{
			return pendingDestroy;
		}

		/// <summary>Sets whether this widget is active.</summary>
		/// <param name="_status">True to activate, false to deactivate.</param>
		virtual void SetActive(const Bool& _status) override;

		/// <summary>Retrieves the attached WidgetComponent of the specified type.</summary>
		/// <typeparam name="Type">WidgetComponent derived type.</typeparam>
		/// <returns>Pointer to component, or nullptr if not attached.</returns>
		template <typename Type, typename ...Args, IS_BASE_OF(WidgetComponent, Type)>
		inline Type* GetComponent()
		{
			for (const std::unique_ptr<WidgetComponent>& _component : components)
				if (Type* _castedComponent = Cast<Type>(_component.get())) return _castedComponent;

			return nullptr;
		}

		/// <summary>Destroys this widget and schedules removal from HUD.</summary>
		virtual void Destroy();

		friend class HUD;
		friend class WidgetComponent;

		/// <summary>Returns a string representation of the widget.</summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;

		/// <summary>Sets widget screen position.</summary>
		/// <param name="_newPosition">Position coordinates.</param>
		void SetPosition(const FVector2& _newPosition) override;
		/// <summary>Gets widget screen position.</summary>
		/// <returns>Position coordinates.</returns>
		FVector2 GetPosition() const override;
		/// <summary>Sets widget rotation angle.</summary>
		/// <param name="_newRotation">Angle.</param>
		void SetRotation(const Angle& _newRotation) override;
		/// <summary>Gets widget rotation angle.</summary>
		/// <returns>Current Angle.</returns>
		Angle GetRotation() const override;
		/// <summary>Sets widget scale multipliers.</summary>
		/// <param name="_newScale">Scale factors.</param>
		void SetScale(const FVector2& _newScale) override;
		/// <summary>Gets widget scale multipliers.</summary>
		/// <returns>Scale factors.</returns>
		FVector2 GetScale() const override;
	};
}
