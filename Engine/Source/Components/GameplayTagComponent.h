#pragma once
#include "Component.h"
#include "GameplayTags/GameplayTag.h"
#include "Reflection/EventList.h"
#include <unordered_map>

namespace Krampus
{
	class Level;

	/// <summary>
	/// The gameplay tags of an actor: what it is and what state it is in ("Enemy.Boss", "State.Stunned"). Choose the starting tags
	/// in the Inspector (tags); scripts and event lists add and remove them while the game runs.
	///
	/// Tags are COUNTED, like Unreal's tag count container: a tag added twice needs to be removed twice. So two effects can both give
	/// "State.Slowed" and the actor stays slowed until both have ended.
	/// Queries follow the hierarchy: an actor with "Enemy.Boss" HasTag "Enemy".
	///
	///     GameplayTagComponent* _tags = GetComponent<GameplayTagComponent>();
	///     _tags->AddTag(GameplayTag::Request("State.Burning"));
	///     if (_tags->HasTag(GameplayTag::Request("State.Burning"))) ...
	///     for (Actor* _enemy : GameplayTagComponent::FindActors(GetLevel(), GameplayTag::Request("Enemy"))) ...
	/// </summary>
	class GameplayTagComponent : public Component
	{
		GameplayTagContainer					tags;
		std::unordered_map<int, int>			counts;		// how many times each tag was added (a tag of the Inspector counts 1)

	public:
		/// <summary>Called after a tag appeared (the first time it was added) / disappeared (its last removal). The event lists of the Inspector are called too.</summary>
		Event<const GameplayTag&>				onTagAdded;
		Event<const GameplayTag&>				onTagRemoved;
		/// <summary>Called after any change of the tags.</summary>
		Event<>									onTagsChanged;

		EventList								onTagAddedCalls;		// functions called when a tag appears (event lists, Inspector)
		EventList								onTagRemovedCalls;

		GameplayTagComponent(Actor* _owner);

		/// <returns>The tags the actor has now.</returns>
		const GameplayTagContainer& GetTags() const { return tags; }

		/// <summary>Replaces the tags (the starting tags of the Inspector, a scene). Does not call the events.</summary>
		void SetTags(const GameplayTagContainer& _tags);

		// ---- Queries (follow the hierarchy)
		bool HasTag(const GameplayTag& _tag) const { return tags.HasTag(_tag); }
		bool HasTagExact(const GameplayTag& _tag) const { return tags.HasTagExact(_tag); }
		bool HasAny(const GameplayTagContainer& _other) const { return tags.HasAny(_other); }
		bool HasAll(const GameplayTagContainer& _other) const { return tags.HasAll(_other); }
		bool Matches(const GameplayTagQuery& _query) const { return _query.Matches(tags); }

		// ---- Changes
		/// <summary>Adds the tag _count times. Returns true when the tag was not there before.</summary>
		bool AddTag(const GameplayTag& _tag, int _count = 1);

		/// <summary>Removes the tag _count times: it goes away when the count reaches 0. Returns true when the tag is gone.</summary>
		bool RemoveTag(const GameplayTag& _tag, int _count = 1);

		/// <summary>Removes the tag whatever its count.</summary>
		bool RemoveTagCompletely(const GameplayTag& _tag);

		/// <summary>Removes every tag.</summary>
		void ClearTags();

		/// <returns>How many times the tag was added (0 when the actor does not have it).</returns>
		int GetTagCount(const GameplayTag& _tag) const;

		// ---- Finding actors

		/// <returns>The tag component of an actor, or nullptr.</returns>
		static GameplayTagComponent* Of(const Actor* _actor);

		/// <returns>True if the actor has the tag (false for an actor without the component).</returns>
		static bool ActorHasTag(const Actor* _actor, const GameplayTag& _tag);

		/// <returns>The actors of the level that have the tag (or a child of it), in the order of the level.</returns>
		static std::vector<Actor*> FindActors(const Level* _level, const GameplayTag& _tag);

		/// <returns>The actors of the level whose tags satisfy the query.</returns>
		static std::vector<Actor*> FindActors(const Level* _level, const GameplayTagQuery& _query);

		std::string ToString() const override;

	private:
		void Changed(const GameplayTag& _tag, bool _added);
	};

}
