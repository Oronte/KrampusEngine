#pragma once
#include "Components/Component.h"
#include "GameFramework/SceneObject.h"

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
	class Actor : public SceneObject<Actor, Component>
	{
		std::uint64_t id = 0;	// unique, saved in scenes: what references to this actor remember

	public:
		explicit Actor(Level* _level);

		/// <summary>
		/// The unique number of this actor: random when it is spawned, read back from the scene file when it is loaded,
		/// so a reference to it (KPROPERTY(Actor*, ...)) finds it again.
		/// </summary>
		[[nodiscard]] std::uint64_t GetId() const { return id; }

		/// <summary>Used by scenes when they load the actor. Two actors of a level must not have the same id.</summary>
		void SetId(const std::uint64_t _id) { id = _id; }

		friend class ActorManager;

	protected:
		void OnDestroy() override;
		void OnComponentRemoved(Component* _component) override;
	};

	extern template class SceneObject<Actor, Component>;

}
