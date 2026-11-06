#include "GameplayTagComponent.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"

using namespace Krampus;

GameplayTagComponent::GameplayTagComponent(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(GameplayTagComponent);
}

void GameplayTagComponent::SetTags(const GameplayTagContainer& _tags)
{
	tags = _tags;
	counts.clear();
	for (const GameplayTag& _tag : tags) counts[_tag.GetIndex()] = 1;
}

int GameplayTagComponent::GetTagCount(const GameplayTag& _tag) const
{
	if (!tags.HasTagExact(_tag)) return 0;
	const auto _found = counts.find(_tag.GetIndex());
	return _found == counts.end() ? 1 : _found->second;
}

bool GameplayTagComponent::AddTag(const GameplayTag& _tag, const int _count)
{
	if (_tag.IsNone() || _count <= 0) return false;
	if (!_tag.IsDeclared())
		LOG_WARNING("GameplayTag: \"" + _tag.GetName() + "\" is not declared (Tools > Gameplay Tags)");

	const bool _isNew = tags.AddTag(_tag);
	counts[_tag.GetIndex()] = (_isNew ? 0 : GetTagCount(_tag)) + _count;
	if (_isNew) Changed(_tag, true);
	return _isNew;
}

bool GameplayTagComponent::RemoveTag(const GameplayTag& _tag, const int _count)
{
	if (_tag.IsNone() || _count <= 0 || !tags.HasTagExact(_tag)) return false;

	const int _left = GetTagCount(_tag) - _count;
	if (_left > 0)
	{
		counts[_tag.GetIndex()] = _left;
		return false;
	}
	return RemoveTagCompletely(_tag);
}

bool GameplayTagComponent::RemoveTagCompletely(const GameplayTag& _tag)
{
	if (!tags.RemoveTag(_tag)) return false;
	counts.erase(_tag.GetIndex());
	Changed(_tag, false);
	return true;
}

void GameplayTagComponent::ClearTags()
{
	const std::vector<GameplayTag> _all = tags.GetTags();
	for (const GameplayTag& _tag : _all) RemoveTagCompletely(_tag);
}

void GameplayTagComponent::Changed(const GameplayTag& _tag, const bool _added)
{
	LOG(VerbosityType::Verbose, "GameplayTag: " + owner->GetName() + (_added ? " + " : " - ") + _tag.GetName());
	if (_added)
	{
		onTagAdded.Broadcast(_tag);
		onTagAddedCalls.Invoke(this);
	}
	else
	{
		onTagRemoved.Broadcast(_tag);
		onTagRemovedCalls.Invoke(this);
	}
	onTagsChanged.Broadcast();
}

std::string GameplayTagComponent::ToString() const
{
	return name + " -> " + tags.ToString();
}

// ---- Finding actors

GameplayTagComponent* GameplayTagComponent::Of(const Actor* _actor)
{
	return _actor ? const_cast<Actor*>(_actor)->GetComponent<GameplayTagComponent>() : nullptr;
}

bool GameplayTagComponent::ActorHasTag(const Actor* _actor, const GameplayTag& _tag)
{
	const GameplayTagComponent* _component = Of(_actor);
	return _component && _component->HasTag(_tag);
}

std::vector<Actor*> GameplayTagComponent::FindActors(const Level* _level, const GameplayTag& _tag)
{
	std::vector<Actor*> _found;
	if (!_level) return _found;
	for (const std::unique_ptr<Actor>& _actor : const_cast<Level*>(_level)->GetActorManager().GetActors())
		if (_actor->IsActive() && !_actor->IsPendingDestroy() && ActorHasTag(_actor.get(), _tag)) _found.push_back(_actor.get());
	return _found;
}

std::vector<Actor*> GameplayTagComponent::FindActors(const Level* _level, const GameplayTagQuery& _query)
{
	std::vector<Actor*> _found;
	if (!_level) return _found;
	for (const std::unique_ptr<Actor>& _actor : const_cast<Level*>(_level)->GetActorManager().GetActors())
	{
		if (!_actor->IsActive() || _actor->IsPendingDestroy()) continue;
		const GameplayTagComponent* _component = Of(_actor.get());
		if (_component && _component->Matches(_query)) _found.push_back(_actor.get());
	}
	return _found;
}
