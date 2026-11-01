#pragma once
#include "components/Component.h"

namespace Krampus
{

	class Level;

	/// <summary>
	/// Sets to nullptr every reflected reference (KPROPERTY(Actor*, ...), KPROPERTY(SpriteComponent*, ...)) of the level that points to
	/// _object, or to a component of _object when it is an actor. Called when an actor is destroyed and when a component is removed.
	/// Defined in Reflection.cpp.
	/// </summary>
	void ForgetReferencesTo(Level* _level, IGameObject* _object);

	/// <summary>
	/// Base class for all interactive entities that can be placed or spawned in a level.
	/// </summary>
	class Actor : public IGameObject, public ITransformable
	{
		std::vector<Actor*>							children;
		Actor*										parent			= nullptr;
		Transform									transform;					// in the world
		Transform									localTransform;				// in the parent (the world one without a parent): what a child keeps
		Bool										pendingDestroy	= false;
		Bool										constructed		= false;	// Construct() ran: the components added from now on are constructed at once
		std::uint64_t								id				= 0;	// unique, saved in scenes: what references to this actor remember
		int											componentLoops	= 0;	// > 0 while the components are walked (ForEachComponent)
		std::vector<Component*>						removedComponents;		// removed during a walk: deleted when it ends

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
			if (!_actor || _actor == this) return;
			for (Actor* _above = parent; _above; _above = _above->parent)
				if (_above == _actor)
				{
					LOG_WARNING("Actor: " + _actor->GetName().StdString() + " is above " + GetName().StdString() + " in the hierarchy, it cannot be its child");
					return;
				}
			if (_actor->parent && _actor->parent != this) _actor->parent->RemoveChild(_actor);	// one parent at a time
			_actor->parent = this;
			_actor->localTransform = RelativeTransform(transform, _actor->transform);	// it stays where it is
			if (std::find(children.begin(), children.end(), _actor) == children.end())
			{
				children.push_back(_actor);
			}
		}

		/// <summary>
		/// The actor this one is a child of, or nullptr.
		/// </summary>
		inline Actor* GetParent() const
		{
			return parent;
		}

		/// <summary>
		/// Gets the child actors in this actor's hierarchy.
		/// </summary>
		inline const std::vector<Actor*>& GetChildren() const
		{
			return children;
		}

		/// <summary>
		/// The unique number of this actor: random when it is spawned, read back from the scene file when it is loaded,
		/// so a reference to it (KPROPERTY(Actor*, ...)) finds it again.
		/// </summary>
		inline std::uint64_t GetId() const
		{
			return id;
		}

		/// <summary>Used by scenes when they load the actor. Two actors of a level must not have the same id.</summary>
		inline void SetId(const std::uint64_t _id)
		{
			id = _id;
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
			if (!_component || IsRemoved(_component)) return;
			LOG(VerbosityType::Verbose, "Component: " + std::string(typeid(*_component).name()) + " removed from " + GetName().StdString());
			_component->Deconstruct();
			_component->BeginDestroy();
			ForgetReferencesTo(level, _component);
			// A component may remove itself (or another one) from its Tick: deleting it now would break the walk of the components.
			if (componentLoops > 0) removedComponents.push_back(_component);
			else std::erase_if(components, [&](const std::unique_ptr<Component>& _item) { return _item.get() == _component; });
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
			if (_toRemove && _toRemove->parent == this)
			{
				_toRemove->parent = nullptr;
				_toRemove->localTransform = _toRemove->transform;
			}
		}

		/// <summary>
		/// Gets the elapsed delta time of the current frame in seconds.
		/// </summary>
		/// <returns>Delta time as a Float.</returns>
		Float GetDeltaTime() const;

		/// <summary>
		/// Constructs an Actor within the specified level.
		/// </summary>
		/// <param name="_level">Pointer to the owning Level.</param>
		Actor(Level* _level);

	protected:
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
			// From the actor's constructor: the actor's Construct() constructs it, once it is placed in the level.
			if (constructed) _rawComponent->Construct();
			LOG(VerbosityType::Verbose, "Component: " + std::string(typeid(Type).name()) + " added to " + GetName().StdString());
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
			RemoveComponentInstance(_component);
		}

		/// <summary>
		/// Calls _function on each component. A component added meanwhile waits for the next walk, a component removed meanwhile is
		/// skipped and deleted when the walk ends: a Tick may add or remove components safely.
		/// </summary>
		template <typename Function>
		inline void ForEachComponent(Function&& _function)
		{
			++componentLoops;
			const size_t _count = components.size();
			for (size_t _i = 0; _i < _count; ++_i)
			{
				Component* _component = components[_i].get();	// read again each time: an addition may move the list
				if (!IsRemoved(_component)) _function(_component);
			}
			if (--componentLoops == 0 && !removedComponents.empty())
			{
				std::erase_if(components, [&](const std::unique_ptr<Component>& _item) { return IsRemoved(_item.get()); });
				removedComponents.clear();
			}
		}

		/// <summary>
		/// True for a component removed during a walk of the components, not deleted yet.
		/// </summary>
		inline bool IsRemoved(const Component* _component) const
		{
			return !removedComponents.empty() && std::find(removedComponents.begin(), removedComponents.end(), _component) != removedComponents.end();
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
			if constexpr (std::is_same_v<Type, Level>)
				return level;
			else
				return Cast<Type>(level);
		}

		/// <summary>
		/// True once Destroy() was called: the actor is waiting to be deleted at the end of the frame.
		/// </summary>
		inline Bool IsPendingDestroy() const
		{
			return pendingDestroy;
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
				if (Type* _castedComponent = Cast<Type>(_component.get()); _castedComponent && !IsRemoved(_component.get())) return _castedComponent;

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

		/// <summary>The position, rotation and scale in the parent (the world ones without a parent).</summary>
		Transform GetLocalTransform() const override { return localTransform; }

		/// <summary>Places the actor in its parent; the children follow.</summary>
		void SetLocalTransform(const Transform& _local) override;

		bool HasParentTransform() const override { return parent != nullptr; }

		inline FVector2 GetLocalPosition() const { return localTransform.position; }
		inline Angle GetLocalRotation() const { return localTransform.rotation; }
		inline FVector2 GetLocalScale() const { return localTransform.scale; }
		inline void SetLocalPosition(const FVector2& _position) { Transform _local = localTransform; _local.position = _position; SetLocalTransform(_local); }
		inline void SetLocalRotation(const Angle& _rotation) { Transform _local = localTransform; _local.rotation = _rotation; SetLocalTransform(_local); }
		inline void SetLocalScale(const FVector2& _scale) { Transform _local = localTransform; _local.scale = _scale; SetLocalTransform(_local); }

	private:
		/// <summary>The world transform was set: the local one is computed again, then the children and the listeners follow.</summary>
		void WorldChanged();

		/// <summary>The world transform comes from the parent and the local one (the parent moved, or the local one was set).</summary>
		void FollowParent();

		/// <summary>The children follow, then onMove / onRotate / onScale for what differs from _old.</summary>
		void Propagate(const Transform& _old);
};

}
