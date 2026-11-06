#pragma once
#include "Core/GameObject.h"
#include "Utilities/System/Transformable.h"
#include "Utilities/System/Timer.h"

namespace Krampus
{
	class Engine;
	class Level;

	/// <summary>
	/// What Actor (world) and Widget (HUD) share: a place in a level, a parent/children hierarchy with world and local
	/// transforms, and a list of components (ComponentType) driven by the object's lifecycle.
	/// CRTP: Self is the derived class. Explicitly instantiated in SceneObject.cpp.
	/// </summary>
	template <typename Self, typename ComponentType>
	class SceneObject : public IGameObject, public ITransformable
	{
		std::vector<Self*>				children;
		Self*							parent			= nullptr;
		Transform						transform;					// in the world (the screen for a widget)
		Transform						localTransform;				// in the parent (the world one without a parent): what a child keeps
		Bool							pendingDestroy	= false;
		Bool							constructed		= false;	// Construct() ran: the components added from now on are constructed at once
		int								componentLoops	= 0;		// > 0 while the components are walked (ForEachComponent)
		std::vector<ComponentType*>		removedComponents;			// removed during a walk: deleted when it ends

	public:
		Event<FVector2>					onMove;
		Event<FVector2>					onScale;
		Event<Angle>					onRotate;

	protected:
		std::vector<std::unique_ptr<ComponentType>>	components;
		Level*										level;

		explicit SceneObject(Level* _level) : level(_level) {}

	public:
		[[nodiscard]] Engine* GetWorld() const;
		[[nodiscard]] Float GetDeltaTime() const;

		template <typename Type = Level> requires std::is_base_of_v<Level, Type>
		[[nodiscard]] Type* GetLevel()
		{
			if constexpr (std::is_same_v<Type, Level>) return level;
			else return Cast<Type>(level);
		}

		// ---- Hierarchy

		/// <summary>Makes _child a child of this object; it keeps its place. Refused when _child is above this one.</summary>
		void AddChild(Self* _child);
		void RemoveChild(Self* _child);
		[[nodiscard]] Self* GetParent() const { return parent; }
		[[nodiscard]] const std::vector<Self*>& GetChildren() const { return children; }

		// ---- Components

		[[nodiscard]] const std::vector<std::unique_ptr<ComponentType>>& GetComponents() const { return components; }

		/// <summary>Public version of CreateComponent, for code that adds components by name (scenes, editor, KCOMPONENT).</summary>
		template <typename Type, typename ...Args> requires std::is_base_of_v<ComponentType, Type>
		Type* AddComponent(Args&&... _args) { return CreateComponent<Type>(std::forward<Args>(_args)...); }

		/// <returns>The first component of that type, or nullptr.</returns>
		template <typename Type> requires std::is_base_of_v<ComponentType, Type>
		[[nodiscard]] Type* GetComponent()
		{
			for (const std::unique_ptr<ComponentType>& _component : components)
				if (Type* _cast = Cast<Type>(_component.get()); _cast && !IsRemoved(_component.get())) return _cast;
			return nullptr;
		}

		/// <summary>Removes a component given by pointer (the editor does not know its type). Safe during a walk of the components.</summary>
		void RemoveComponentInstance(ComponentType* _component);

		// ---- Lifecycle

		/// <summary>True once Destroy() was called: the object is waiting to be deleted at the end of the frame.</summary>
		[[nodiscard]] Bool IsPendingDestroy() const { return pendingDestroy; }

		/// <summary>Deconstructs the object and leaves the hierarchy (the children stay, without a parent); deleted at the end of the frame.</summary>
		virtual void Destroy();

		void SetActive(const Bool& _status) override;
		std::string ToString() const override;

		// ---- Transform (setting the world one moves the children; the events fire for what changed)

		void SetPosition(const FVector2& _position) override;
		FVector2 GetPosition() const override { return transform.position; }
		void SetRotation(const Angle& _rotation) override;
		Angle GetRotation() const override { return transform.rotation; }
		void SetScale(const FVector2& _scale) override;
		FVector2 GetScale() const override { return transform.scale; }

		/// <summary>The position, rotation and scale in the parent (the world ones without a parent).</summary>
		Transform GetLocalTransform() const override { return localTransform; }
		/// <summary>Places the object in its parent; the children follow.</summary>
		void SetLocalTransform(const Transform& _local) override;
		bool HasParentTransform() const override { return parent != nullptr; }

		[[nodiscard]] FVector2 GetLocalPosition() const	{ return localTransform.position; }
		[[nodiscard]] Angle GetLocalRotation() const		{ return localTransform.rotation; }
		[[nodiscard]] FVector2 GetLocalScale() const		{ return localTransform.scale; }
		void SetLocalPosition(const FVector2& _position)	{ Transform _local = localTransform; _local.position = _position; SetLocalTransform(_local); }
		void SetLocalRotation(const Angle& _rotation)		{ Transform _local = localTransform; _local.rotation = _rotation; SetLocalTransform(_local); }
		void SetLocalScale(const FVector2& _scale)			{ Transform _local = localTransform; _local.scale = _scale; SetLocalTransform(_local); }

		template <typename> friend class OwnedComponent;

	protected:
		/// <summary>Creates and attaches a component (one per class). Constructed at once if this object already is, else by Construct().</summary>
		/// <returns>The new component, or nullptr when one of that class exists.</returns>
		template <typename Type, typename ...Args> requires std::is_base_of_v<ComponentType, Type>
		Type* CreateComponent(Args&&... _args)
		{
			if (GetComponent<Type>())
			{
				LOG_ERROR(std::string("There is already a component of type ") + typeid(Type).name() + " on " + GetName());
				return nullptr;
			}
			std::unique_ptr<Type> _component = std::make_unique<Type>(static_cast<Self*>(this), std::forward<Args>(_args)...);
			Type* _raw = _component.get();
			components.push_back(std::move(_component));
			if (constructed) _raw->Construct();
			LOG(VerbosityType::Verbose, "Component: " + std::string(typeid(Type).name()) + " added to " + GetName());
			return _raw;
		}

		template <typename Type> requires std::is_base_of_v<ComponentType, Type>
		void RemoveComponent()
		{
			if (Type* _component = GetComponent<Type>()) RemoveComponentInstance(_component);
			else LOG_WARNING(std::string("There is no component of type ") + typeid(Type).name() + " on " + GetName());
		}

		/// <summary>
		/// Calls _function on each component. A component added meanwhile waits for the next walk, a component removed meanwhile is
		/// skipped and deleted when the walk ends: a Tick may add or remove components safely.
		/// </summary>
		template <typename Function>
		void ForEachComponent(Function&& _function)
		{
			++componentLoops;
			const size_t _count = components.size();
			for (size_t _i = 0; _i < _count; ++_i)
			{
				ComponentType* _component = components[_i].get();	// read again each time: an addition may move the list
				if (!IsRemoved(_component)) _function(_component);
			}
			if (--componentLoops == 0 && !removedComponents.empty())
			{
				std::erase_if(components, [&](const std::unique_ptr<ComponentType>& _item) { return IsRemoved(_item.get()); });
				removedComponents.clear();
			}
		}

		/// <summary>Creates a timer ticked by the level.</summary>
		Timer* CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);

		void Construct() override;
		void Deconstruct() override;
		void BeginPlay() override;
		void Tick(const Float& _deltaTime) override;
		void BeginDestroy() override;

		/// <summary>Destroy(): hands the object to its manager for deletion and clears what points to it.</summary>
		virtual void OnDestroy() = 0;

		/// <summary>A component is being removed: clears what points to it.</summary>
		virtual void OnComponentRemoved(ComponentType*) {}

	private:
		[[nodiscard]] bool IsRemoved(const ComponentType* _component) const
		{
			return !removedComponents.empty() && std::find(removedComponents.begin(), removedComponents.end(), _component) != removedComponents.end();
		}

		/// <summary>The world transform was set: the local one is computed again, then the children follow.</summary>
		void WorldChanged();
		/// <summary>The world transform comes from the parent and the local one.</summary>
		void FollowParent();
		/// <summary>The children follow, then onMove / onRotate / onScale for what differs from _old.</summary>
		void Propagate(const Transform& _old);
	};

}
