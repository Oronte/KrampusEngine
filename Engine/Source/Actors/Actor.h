#pragma once
#include "components/Component.h"

namespace Krampus
{

	class Level;

	/// <summary>
	/// Base class for all interactive entities that can be placed or spawned in a level.
	/// </summary>
	class Actor : public IGameObject, public ITransformable
	{
		std::vector<Actor*>							children;
		Transform									transform;
	
	public:
		/// <summary>
		/// Event broadcast when the actor's position changes.
		/// </summary>
		Event<FVector2>								onMove;

		/// <summary>
		/// Event broadcast when the actor's scale changes.
		/// </summary>
		Event<FVector2>								onScale;

		/// <summary>
		/// Event broadcast when the actor's rotation changes.
		/// </summary>
		Event<Angle>								onRotate;

	protected:
		std::vector<std::unique_ptr<Component>>		components;
		Level*										level;

	public:
		/// <summary>
		/// Gets a pointer to the Engine world that owns the actor's level.
		/// </summary>
		/// <returns>Pointer to the Engine world.</returns>
		Engine* GetWorld() const;

		/// <summary>
		/// Adds a child actor to this actor's hierarchy.
		/// </summary>
		/// <param name="_actor">Pointer to the child actor to add.</param>
		inline void AddChild(Actor* _actor)
		{
			if (std::find(children.begin(), children.end(), _actor) == children.end())
			{
				children.push_back(_actor);
			}
		}

		/// <summary>
		/// Gets the child actors in this actor's hierarchy.
		/// </summary>
		inline const std::vector<Actor*>& GetChildren() const
		{
			return children;
		}

		/// <summary>
		/// Gets the components attached to this actor.
		/// </summary>
		inline const std::vector<std::unique_ptr<Component>>& GetComponents() const
		{
			return components;
		}

		/// <summary>
		/// Public version of CreateComponent, for code that adds components by name (scenes, editor, KCOMPONENT).
		/// </summary>
		template <typename Type, typename ...Args, IS_BASE_OF(Component, Type)>
		inline Type* AddComponent(Args&&... _args)
		{
			return CreateComponent<Type>(std::forward<Args>(_args)...);
		}

		/// <summary>
		/// Removes a component given by pointer (the editor does not know its type).
		/// </summary>
		inline void RemoveComponentInstance(Component* _component)
		{
			if (!_component) return;
			LOG(VerbosityType::Log, "Component: " + std::string(typeid(*_component).name()) + " removed from " + GetName().StdString());
			_component->Deconstruct();
			_component->BeginDestroy();
			std::erase_if(components, [&](const std::unique_ptr<Component>& _item) { return _item.get() == _component; });
		}

		/// <summary>
		/// Removes a child actor from this actor's hierarchy.
		/// </summary>
		/// <param name="_toRemove">Pointer to the child actor to remove.</param>
		inline void RemoveChild(Actor* _toRemove)
		{
			std::erase_if(children, [&](Actor* _actor)
				{
					return _actor == _toRemove;
				});
		}

		/// <summary>
		/// Gets the elapsed delta time of the current frame in seconds.
		/// </summary>
		/// <returns>Delta time as a Float.</returns>
		Float GetDeltaTime() const;

	protected:
		/// <summary>
		/// Constructs an Actor within the specified level.
		/// </summary>
		/// <param name="_level">Pointer to the owning Level.</param>
		Actor(Level* _level);

		/// <summary>
		/// Creates and attaches a component of the specified type to this actor.
		/// </summary>
		/// <typeparam name="Type">The component type derived from Component.</typeparam>
		/// <param name="_args">Arguments forwarded to the component constructor.</param>
		/// <returns>Pointer to the newly created component, or nullptr if a component of this type already exists.</returns>
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
			LOG(VerbosityType::Log, "Component: " + std::string(typeid(Type).name()) + " added to " + GetName().StdString());
			return _rawComponent;
		}

		/// <summary>
		/// Removes a component of the specified type from this actor.
		/// </summary>
		/// <typeparam name="Type">The component type to remove.</typeparam>
		template <typename Type, IS_BASE_OF(Component, Type)>
		inline void RemoveComponent()
		{
			Type* _component = GetComponent<Type>();
			if (!_component)
			{
				LOG_WARNING("There is not a component of type" + std::string(NAME_OF(Type)));
				return;
			}

			LOG(VerbosityType::Log, "Component: " + std::string(typeid(Type).name()) + " removed from " + GetName().StdString());
			_component->Deconstruct();
			_component->BeginDestroy();

			std::erase_if(components,
				[&](const std::unique_ptr<Component>& _item)
				{
					return _item.get() == _component;
				});
		}

		/// <summary>
		/// Creates and starts a timer managed within the actor's level.
		/// </summary>
		/// <param name="_callback">Function to call when the timer expires.</param>
		/// <param name="_duration">Duration of the timer in seconds.</param>
		/// <param name="_isLoop">Whether the timer should repeat automatically.</param>
		/// <param name="_startRunning">Whether the timer starts running immediately.</param>
		/// <returns>Pointer to the created Timer.</returns>
		Timer* CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);

		/// <summary>
		/// Lifecycle callback invoked when the actor is constructed.
		/// </summary>
		void Construct() override;

		/// <summary>
		/// Lifecycle callback invoked when the actor is deconstructed.
		/// </summary>
		void Deconstruct() override;

		/// <summary>
		/// Lifecycle callback invoked when gameplay starts for this actor.
		/// </summary>
		void BeginPlay() override;

		/// <summary>
		/// Updates the actor each frame.
		/// </summary>
		/// <param name="_deltaTime">Time elapsed since the previous frame in seconds.</param>
		void Tick(const Float& _deltaTime) override;

		/// <summary>
		/// Lifecycle callback invoked just before the actor is destroyed.
		/// </summary>
		void BeginDestroy() override;
		
	public:
		/// <summary>
		/// Gets the owning Level cast to the requested type.
		/// </summary>
		/// <typeparam name="Type">The level type derived from Level.</typeparam>
		/// <returns>Pointer to the cast Level instance.</returns>
		template<typename Type = Level, IS_BASE_OF(Level, Type)>
		inline Type* GetLevel()
		{
			if (InstanceOf<Type>(level))
			{
				return level;
			}

			return Cast<Type>(level);
		}

		/// <summary>
		/// Sets whether this actor is active in the world.
		/// </summary>
		/// <param name="_status">True to activate, false to deactivate.</param>
		virtual void SetActive(const Bool& _status) override;

		/// <summary>
		/// Retrieves the first attached component of the specified type.
		/// </summary>
		/// <typeparam name="Type">The component type derived from Component.</typeparam>
		/// <returns>Pointer to the component if found, nullptr otherwise.</returns>
		template <typename Type, typename ...Args, IS_BASE_OF(Component, Type)>
		inline Type* GetComponent()
		{
			for (const std::unique_ptr<Component>& _component : components)
				if (Type* _castedComponent = Cast<Type>(_component.get())) return _castedComponent;

			return nullptr;
		}

		/// <summary>
		/// Destroys this actor and marks it for removal from the level.
		/// </summary>
		virtual void Destroy();

		friend class ActorManager;
		friend class Component;

		/// <summary>
		/// Returns a string representation of the actor.
		/// </summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;

		/// <summary>
		/// Sets the world position of the actor.
		/// </summary>
		/// <param name="_newPosition">The new position coordinates.</param>
		void SetPosition(const FVector2& _newPosition) override;

		/// <summary>
		/// Gets the world position of the actor.
		/// </summary>
		/// <returns>The actor's position vector.</returns>
		FVector2 GetPosition() const override;

		/// <summary>
		/// Sets the rotation angle of the actor.
		/// </summary>
		/// <param name="_newRotation">The new angle.</param>
		void SetRotation(const Angle& _newRotation) override;

		/// <summary>
		/// Gets the rotation angle of the actor.
		/// </summary>
		/// <returns>The current rotation Angle.</returns>
		Angle GetRotation() const override;

		/// <summary>
		/// Sets the 2D scale factor of the actor.
		/// </summary>
		/// <param name="_newScale">The new scale vector.</param>
		void SetScale(const FVector2& _newScale) override;

		/// <summary>
		/// Gets the 2D scale factor of the actor.
		/// </summary>
		/// <returns>The scale vector.</returns>
		FVector2 GetScale() const override;
};

}
