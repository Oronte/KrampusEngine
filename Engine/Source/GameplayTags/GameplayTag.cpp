#include "GameplayTag.h"
#include "Core/AssetPack.h"
#include "Utilities/Macro.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>

using namespace Krampus;

// ---- The table every tag points into

namespace
{
	struct Entry
	{
		std::string		name;
		int				parent		= -1;
		int				depth		= 0;
		bool			declared	= false;
	};

	struct Table
	{
		std::vector<Entry>						entries;
		std::unordered_map<std::string, int>	index;
		bool									loaded		= false;
		int										version		= 0;
	};

	Table& GetTable()
	{
		static Table _table;
		return _table;
	}

	// "A.B.C" -> the number of the tag, made (with its parents) when it is not there yet. -1 if the name is not valid.
	int Intern(const std::string& _name)
	{
		if (!GameplayTag::IsValidName(_name)) return -1;

		Table& _table = GetTable();
		if (const auto _found = _table.index.find(_name); _found != _table.index.end()) return _found->second;

		int _parent = -1;
		if (const size_t _dot = _name.rfind('.'); _dot != std::string::npos)
			_parent = Intern(_name.substr(0, _dot));

		Entry _entry;
		_entry.name = _name;
		_entry.parent = _parent;
		_entry.depth = _parent < 0 ? 0 : _table.entries[_parent].depth + 1;
		_table.entries.push_back(std::move(_entry));
		const int _id = static_cast<int>(_table.entries.size()) - 1;
		_table.index[_name] = _id;
		return _id;
	}

	std::string Trim(const std::string& _text)
	{
		const size_t _begin = _text.find_first_not_of(" \t\r\n");
		if (_begin == std::string::npos) return "";
		return _text.substr(_begin, _text.find_last_not_of(" \t\r\n") - _begin + 1);
	}

	std::vector<std::string> Split(const std::string& _text, const char _separator)
	{
		std::vector<std::string> _parts;
		size_t _start = 0;
		for (size_t _end = _text.find(_separator); _end != std::string::npos; _end = _text.find(_separator, _start))
		{
			_parts.push_back(_text.substr(_start, _end - _start));
			_start = _end + 1;
		}
		_parts.push_back(_text.substr(_start));
		return _parts;
	}
}

// ---- GameplayTag

bool GameplayTag::IsValidName(const std::string& _name)
{
	if (_name.empty() || _name.front() == '.' || _name.back() == '.') return false;
	char _previous = 0;
	for (const char _c : _name)
	{
		if (!(std::isalnum(static_cast<unsigned char>(_c)) || _c == '_' || _c == '.')) return false;
		if (_c == '.' && _previous == '.') return false;
		_previous = _c;
	}
	return true;
}

GameplayTag GameplayTag::Request(const std::string& _name)
{
	GameplayTagsManager::EnsureLoaded();
	return FromIndex(Intern(Trim(_name)));
}

bool GameplayTag::IsDeclared() const
{
	return id >= 0 && GetTable().entries[id].declared;
}

const std::string& GameplayTag::GetName() const
{
	static const std::string _none;
	return id < 0 ? _none : GetTable().entries[id].name;
}

std::string GameplayTag::GetLeafName() const
{
	const std::string& _name = GetName();
	const size_t _dot = _name.rfind('.');
	return _dot == std::string::npos ? _name : _name.substr(_dot + 1);
}

GameplayTag GameplayTag::GetParent() const
{
	return id < 0 ? GameplayTag() : FromIndex(GetTable().entries[id].parent);
}

int GameplayTag::GetDepth() const
{
	return id < 0 ? -1 : GetTable().entries[id].depth;
}

std::vector<GameplayTag> GameplayTag::GetWithParents() const
{
	std::vector<GameplayTag> _tags;
	for (int _walk = id; _walk >= 0; _walk = GetTable().entries[_walk].parent)
		_tags.push_back(FromIndex(_walk));
	return _tags;
}

bool GameplayTag::MatchesTag(const GameplayTag& _other) const
{
	if (_other.id < 0) return false;
	const Table& _table = GetTable();
	for (int _walk = id; _walk >= 0; _walk = _table.entries[_walk].parent)
		if (_walk == _other.id) return true;
	return false;
}

// ---- GameplayTagContainer

bool GameplayTagContainer::AddTag(const GameplayTag& _tag)
{
	if (_tag.IsNone()) return false;
	const auto _place = std::lower_bound(tags.begin(), tags.end(), _tag);
	if (_place != tags.end() && *_place == _tag) return false;
	tags.insert(_place, _tag);
	return true;
}

void GameplayTagContainer::AppendTags(const GameplayTagContainer& _other)
{
	for (const GameplayTag& _tag : _other.tags) AddTag(_tag);
}

bool GameplayTagContainer::RemoveTag(const GameplayTag& _tag)
{
	const auto _found = std::find(tags.begin(), tags.end(), _tag);
	if (_found == tags.end()) return false;
	tags.erase(_found);
	return true;
}

void GameplayTagContainer::RemoveTags(const GameplayTagContainer& _other)
{
	for (const GameplayTag& _tag : _other.tags) RemoveTag(_tag);
}

bool GameplayTagContainer::HasTag(const GameplayTag& _tag) const
{
	if (_tag.IsNone()) return false;
	for (const GameplayTag& _mine : tags)
		if (_mine.MatchesTag(_tag)) return true;
	return false;
}

bool GameplayTagContainer::HasTagExact(const GameplayTag& _tag) const
{
	return !_tag.IsNone() && std::find(tags.begin(), tags.end(), _tag) != tags.end();
}

bool GameplayTagContainer::HasAny(const GameplayTagContainer& _other) const
{
	for (const GameplayTag& _tag : _other.tags)
		if (HasTag(_tag)) return true;
	return false;
}

bool GameplayTagContainer::HasAll(const GameplayTagContainer& _other) const
{
	for (const GameplayTag& _tag : _other.tags)
		if (!HasTag(_tag)) return false;
	return true;
}

bool GameplayTagContainer::HasAnyExact(const GameplayTagContainer& _other) const
{
	for (const GameplayTag& _tag : _other.tags)
		if (HasTagExact(_tag)) return true;
	return false;
}

bool GameplayTagContainer::HasAllExact(const GameplayTagContainer& _other) const
{
	for (const GameplayTag& _tag : _other.tags)
		if (!HasTagExact(_tag)) return false;
	return true;
}

GameplayTagContainer GameplayTagContainer::Filter(const GameplayTagContainer& _other) const
{
	GameplayTagContainer _result;
	for (const GameplayTag& _mine : tags)
		if (_other.HasTag(_mine)) _result.AddTag(_mine);
	return _result;
}

GameplayTagContainer GameplayTagContainer::GetWithParents() const
{
	GameplayTagContainer _result;
	for (const GameplayTag& _mine : tags)
		for (const GameplayTag& _tag : _mine.GetWithParents()) _result.AddTag(_tag);
	return _result;
}

std::string GameplayTagContainer::Encode() const
{
	std::string _text;
	for (const GameplayTag& _tag : tags)
	{
		if (!_text.empty()) _text += ';';
		_text += _tag.GetName();
	}
	return _text;
}

GameplayTagContainer GameplayTagContainer::Decode(const std::string& _text)
{
	GameplayTagContainer _container;
	if (_text.empty()) return _container;
	for (const std::string& _name : Split(_text, ';'))
		_container.AddTag(GameplayTag::Request(_name));
	return _container;
}

std::string GameplayTagContainer::ToString() const
{
	std::string _text = "(";
	for (size_t _i = 0; _i < tags.size(); ++_i)
		_text += (_i ? ", " : "") + tags[_i].GetName();
	return _text + ")";
}

// ---- GameplayTagQuery

bool GameplayTagQuery::Matches(const GameplayTagContainer& _tags) const
{
	if (!_tags.HasAll(allTags)) return false;
	if (!anyTags.IsEmpty() && !_tags.HasAny(anyTags)) return false;
	if (_tags.HasAny(noneTags)) return false;
	return true;
}

std::string GameplayTagQuery::Encode() const
{
	if (IsEmpty()) return "";
	return "all=" + allTags.Encode() + "|any=" + anyTags.Encode() + "|none=" + noneTags.Encode();
}

GameplayTagQuery GameplayTagQuery::Decode(const std::string& _text)
{
	GameplayTagQuery _query;
	for (const std::string& _part : Split(_text, '|'))
	{
		const size_t _equal = _part.find('=');
		if (_equal == std::string::npos) continue;
		const std::string _key = _part.substr(0, _equal);
		const GameplayTagContainer _tags = GameplayTagContainer::Decode(_part.substr(_equal + 1));
		if (_key == "all") _query.allTags = _tags;
		else if (_key == "any") _query.anyTags = _tags;
		else if (_key == "none") _query.noneTags = _tags;
	}
	return _query;
}

// ---- GameplayTagsManager

std::string GameplayTagsManager::GetPath()
{
	return std::string(CONTENT_PATH) + "Settings/GameplayTags.cfg";
}

void GameplayTagsManager::EnsureLoaded()
{
	Table& _table = GetTable();
	if (_table.loaded) return;
	_table.loaded = true;		// first: AddTag below asks for the table again

	std::unique_ptr<std::istream> _stream = AssetPack::OpenStream(std::filesystem::path(GetPath()));	// the pak of a packed game, else the disk
	if (!_stream) return;

	std::string _line;
	while (std::getline(*_stream, _line))
	{
		_line = Trim(_line);
		if (_line.empty() || _line[0] == '#') continue;
		AddTag(_line);
	}
	_table.version = 0;
}

void GameplayTagsManager::Reload()
{
	Table& _table = GetTable();
	for (Entry& _entry : _table.entries) _entry.declared = false;
	_table.loaded = false;
	EnsureLoaded();
	++_table.version;
}

GameplayTag GameplayTagsManager::AddTag(const std::string& _name)
{
	EnsureLoaded();
	const int _id = Intern(Trim(_name));
	if (_id < 0) return GameplayTag();

	Table& _table = GetTable();
	for (int _walk = _id; _walk >= 0; _walk = _table.entries[_walk].parent)
		_table.entries[_walk].declared = true;
	++_table.version;
	return GameplayTag::FromIndex(_id);
}

void GameplayTagsManager::RemoveTag(const GameplayTag& _tag)
{
	EnsureLoaded();
	if (_tag.IsNone()) return;

	Table& _table = GetTable();
	for (size_t _i = 0; _i < _table.entries.size(); ++_i)
		if (_table.entries[_i].declared && GameplayTag::FromIndex(static_cast<int>(_i)).MatchesTag(_tag))
			_table.entries[_i].declared = false;
	++_table.version;
}

std::vector<GameplayTag> GameplayTagsManager::GetAllTags()
{
	EnsureLoaded();
	std::vector<GameplayTag> _tags;
	const Table& _table = GetTable();
	for (size_t _i = 0; _i < _table.entries.size(); ++_i)
		if (_table.entries[_i].declared) _tags.push_back(GameplayTag::FromIndex(static_cast<int>(_i)));
	std::sort(_tags.begin(), _tags.end());
	return _tags;
}

std::vector<GameplayTag> GameplayTagsManager::GetChildren(const GameplayTag& _parent)
{
	EnsureLoaded();
	std::vector<GameplayTag> _tags;
	const Table& _table = GetTable();
	for (size_t _i = 0; _i < _table.entries.size(); ++_i)
		if (_table.entries[_i].declared && _table.entries[_i].parent == _parent.GetIndex()) _tags.push_back(GameplayTag::FromIndex(static_cast<int>(_i)));
	std::sort(_tags.begin(), _tags.end());
	return _tags;
}

int GameplayTagsManager::GetVersion()
{
	return GetTable().version;
}

bool GameplayTagsManager::Save()
{
	EnsureLoaded();
	std::error_code _error;
	std::filesystem::create_directories(std::filesystem::path(GetPath()).parent_path(), _error);

	std::ofstream _file(GetPath(), std::ios::trunc);
	if (!_file) return false;
	_file << "# Gameplay tags of the project, one per line. Edit them in the editor: Tools > Gameplay Tags.\n";
	for (const GameplayTag& _tag : GetAllTags())
		_file << _tag.GetName() << "\n";
	return _file.good();
}
