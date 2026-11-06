#pragma once
#include <algorithm>
#include <string>
#include <vector>

// Gameplay tags, like Unreal's: hierarchical names ("Status.Debuff.Burning") that describe what something is or has, without code
// and without a class for each idea. The tags of the project are declared in Content/Settings/GameplayTags.cfg (Tools > Gameplay Tags).
//
//     GameplayTag     one tag. "Status.Debuff.Burning" MATCHES "Status.Debuff" and "Status" (its parents), not the other way round.
//     GameplayTagContainer   a set of tags: HasTag (parents match), HasAny, HasAll, HasTagExact...
//     GameplayTagQuery       "all of these, any of these, none of these" on a container.
//     GameplayTagsManager    the declared tags: add, remove, save, list.
//     GameplayTagComponent   the tags an actor has (Components/GameplayTagComponent.h).
//
// In a script:   KPROPERTY(GameplayTag, enemyKind);   KPROPERTY(GameplayTagContainer, immunities);
//                if (tags->HasTag(GameplayTag::Request("State.Stunned"))) return;

namespace Krampus
{

	/// <summary>
	/// A tag: a name in a table shared by the whole game (so comparing two tags is comparing two numbers).
	/// A default tag is "None".
	/// </summary>
	class GameplayTag
	{
		int		id		= -1;

	public:
		GameplayTag() = default;

		/// <summary>So that a name can be written where a tag is expected: HasTag("Team.Enemy"), AddTag("Status.Burning").</summary>
		GameplayTag(const char* _name) : GameplayTag(Request(_name)) { }
		GameplayTag(const std::string& _name) : GameplayTag(Request(_name)) { }

		/// <summary>
		/// The tag with this name ("A.B.C": words made of letters, digits and _ separated by dots). A name that is not valid gives None.
		/// The tag does not have to be declared (IsDeclared tells), so a scene can carry a tag the project no longer lists.
		/// </summary>
		static GameplayTag Request(const std::string& _name);

		/// <returns>False for None.</returns>
		bool IsNone() const { return id < 0; }

		/// <returns>True when the tag is declared in the project (Content/Settings/GameplayTags.cfg).</returns>
		bool IsDeclared() const;

		/// <returns>"Status.Debuff.Burning", or "" for None.</returns>
		const std::string& GetName() const;

		/// <returns>The last word: "Burning".</returns>
		std::string GetLeafName() const;

		/// <returns>"Status.Debuff" for "Status.Debuff.Burning"; None for a tag without a parent.</returns>
		GameplayTag GetParent() const;

		/// <returns>Number of dots: 0 for "Status", 2 for "Status.Debuff.Burning".</returns>
		int GetDepth() const;

		/// <summary>The tag and all its parents, the tag first.</summary>
		std::vector<GameplayTag> GetWithParents() const;

		/// <summary>True when this tag is _other or one of its children: "A.B.C" matches "A.B", "A.B" does not match "A.B.C".</summary>
		bool MatchesTag(const GameplayTag& _other) const;

		/// <summary>True when the two tags are the same.</summary>
		bool MatchesTagExact(const GameplayTag& _other) const { return id == _other.id && id >= 0; }

		bool operator==(const GameplayTag& _other) const { return id == _other.id; }
		bool operator!=(const GameplayTag& _other) const { return id != _other.id; }
		bool operator<(const GameplayTag& _other) const { return GetName() < _other.GetName(); }

		/// <returns>True if _name can be a tag name.</returns>
		static bool IsValidName(const std::string& _name);

		/// <summary>The number of the tag in the shared table (-1 = None). Used by the tags themselves; prefer Request.</summary>
		int GetIndex() const { return id; }
		static GameplayTag FromIndex(const int _index) { GameplayTag _tag; _tag.id = _index; return _tag; }
	};

	/// <summary>
	/// A set of tags. The queries follow the hierarchy: a container with "A.B.C" HasTag "A.B" (and "A"), the Exact versions do not.
	/// </summary>
	class GameplayTagContainer
	{
		std::vector<GameplayTag>	tags;		// no doubles, sorted by name

	public:
		GameplayTagContainer() = default;
		GameplayTagContainer(const GameplayTag& _tag) { AddTag(_tag); }

		/// <summary>Adds a tag. Returns false when it was already there (or None).</summary>
		bool AddTag(const GameplayTag& _tag);

		/// <summary>Adds the tags of another container.</summary>
		void AppendTags(const GameplayTagContainer& _other);

		/// <summary>Removes the tag itself (not its children). Returns false when it was not there.</summary>
		bool RemoveTag(const GameplayTag& _tag);

		/// <summary>Removes the tags that are in _other.</summary>
		void RemoveTags(const GameplayTagContainer& _other);

		void Reset() { tags.clear(); }
		bool IsEmpty() const { return tags.empty(); }
		int Num() const { return static_cast<int>(tags.size()); }
		const std::vector<GameplayTag>& GetTags() const { return tags; }
		std::vector<GameplayTag>::const_iterator begin() const { return tags.begin(); }
		std::vector<GameplayTag>::const_iterator end() const { return tags.end(); }

		/// <summary>True if one of my tags is _tag or a child of it.</summary>
		bool HasTag(const GameplayTag& _tag) const;
		/// <summary>True if _tag is in the container.</summary>
		bool HasTagExact(const GameplayTag& _tag) const;
		/// <summary>True if HasTag is true for at least one tag of _other (false for an empty _other).</summary>
		bool HasAny(const GameplayTagContainer& _other) const;
		/// <summary>True if HasTag is true for every tag of _other (true for an empty _other).</summary>
		bool HasAll(const GameplayTagContainer& _other) const;
		bool HasAnyExact(const GameplayTagContainer& _other) const;
		bool HasAllExact(const GameplayTagContainer& _other) const;

		/// <returns>My tags that match one of _other (a tag below one of its tags counts).</returns>
		GameplayTagContainer Filter(const GameplayTagContainer& _other) const;

		/// <returns>My tags and all their parents.</returns>
		GameplayTagContainer GetWithParents() const;

		/// <summary>"A.B;C.D" (what scenes and the Inspector store).</summary>
		std::string Encode() const;
		static GameplayTagContainer Decode(const std::string& _text);

		/// <summary>"(A.B, C.D)", for the log.</summary>
		std::string ToString() const;

		bool operator==(const GameplayTagContainer& _other) const { return tags == _other.tags; }
		bool operator!=(const GameplayTagContainer& _other) const { return tags != _other.tags; }
	};

	/// <summary>
	/// A condition on a container: it must have all of allTags, at least one of anyTags (when there are some) and none of noneTags.
	/// Empty parts do not count.
	/// </summary>
	struct GameplayTagQuery
	{
		GameplayTagContainer	allTags;
		GameplayTagContainer	anyTags;
		GameplayTagContainer	noneTags;

		bool Matches(const GameplayTagContainer& _tags) const;
		bool IsEmpty() const { return allTags.IsEmpty() && anyTags.IsEmpty() && noneTags.IsEmpty(); }

		/// <summary>"all=A;B|any=C|none=D".</summary>
		std::string Encode() const;
		static GameplayTagQuery Decode(const std::string& _text);
	};

	/// <summary>
	/// The tags of the project. They live in Content/Settings/GameplayTags.cfg (one per line, "#" for a comment), read the first
	/// time they are needed, also from the pak of a packaged game. Declaring "A.B.C" declares "A.B" and "A" too.
	/// </summary>
	class GameplayTagsManager
	{
	public:
		/// <summary>Reads the file (once). Called by everything that needs the list.</summary>
		static void EnsureLoaded();

		/// <summary>Reads the file again (after it was changed on the disk).</summary>
		static void Reload();

		/// <summary>Writes Content/Settings/GameplayTags.cfg. Returns false if the file cannot be written.</summary>
		static bool Save();

		/// <summary>Declares a tag and its parents. Returns the tag, or None if the name is not valid.</summary>
		static GameplayTag AddTag(const std::string& _name);

		/// <summary>Stops declaring a tag and everything below it. The tags stay in the scenes that use them (shown in red by the editor).</summary>
		static void RemoveTag(const GameplayTag& _tag);

		/// <returns>Every declared tag, sorted by name.</returns>
		static std::vector<GameplayTag> GetAllTags();

		/// <returns>The declared tags just below _parent (None = the roots), sorted.</returns>
		static std::vector<GameplayTag> GetChildren(const GameplayTag& _parent);

		/// <returns>Changes each time a tag is added or removed (an editor refreshes its lists when it moves).</returns>
		static int GetVersion();

		/// <returns>The path of the file.</returns>
		static std::string GetPath();
	};

}
