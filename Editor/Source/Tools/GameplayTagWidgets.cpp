#include "GameplayTagWidgets.h"
#include "Actors/Actor.h"
#include "Components/GameplayTagComponent.h"
#include "GameFramework/Level.h"

using namespace Krampus;

namespace
{
	const ImVec4 UNDECLARED = ImVec4(1.f, 0.45f, 0.3f, 1.f);	// a tag that the project does not declare

	std::string Lower(std::string _text)
	{
		std::transform(_text.begin(), _text.end(), _text.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
		return _text;
	}

	// The tags below _parent as a tree: a click on a name picks it. Returns true when one was picked (its name in _picked).
	bool DrawTagTree(const GameplayTag& _parent, std::string& _picked)
	{
		bool _done = false;
		for (const GameplayTag& _tag : GameplayTagsManager::GetChildren(_parent))
		{
			const bool _hasChildren = !GameplayTagsManager::GetChildren(_tag).empty();
			ImGui::PushID(_tag.GetIndex());
			ImGuiTreeNodeFlags _flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
			if (!_hasChildren) _flags |= ImGuiTreeNodeFlags_Leaf;
			const bool _open = ImGui::TreeNodeEx(_tag.GetLeafName().c_str(), _flags);
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
			{
				_picked = _tag.GetName();
				_done = true;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", _tag.GetName().c_str());
			if (_open)
			{
				if (_hasChildren && DrawTagTree(_tag, _picked)) _done = true;
				ImGui::TreePop();
			}
			ImGui::PopID();
			if (_done) break;
		}
		return _done;
	}

	// The inside of the popup that lists the tags. Returns true when one was chosen (_picked = its name, "" for None).
	bool DrawPickerBody(std::string& _picked, const bool _allowNone)
	{
		static char _filter[64] = "";
		if (ImGui::IsWindowAppearing()) { _filter[0] = 0; ImGui::SetKeyboardFocusHere(); }
		ImGui::SetNextItemWidth(260.f);
		ImGui::InputTextWithHint("##tagfilter", "Search a tag...", _filter, sizeof(_filter));

		bool _done = false;

		// A name that does not exist yet: one click declares it (saved in the file) and picks it, no need to open the Tags window.
		const std::string _typed = _filter;
		if (!_typed.empty() && GameplayTag::IsValidName(_typed) && !GameplayTag::Request(_typed).IsDeclared())
		{
			if (ImGui::Button(("+ Create \"" + _typed + "\"").c_str(), ImVec2(300.f, 0.f)) || ImGui::IsKeyPressed(ImGuiKey_Enter))
			{
				GameplayTagsManager::AddTag(_typed);
				GameplayTagsManager::Save();
				_picked = _typed;
				return true;
			}
		}
		ImGui::BeginChild("##taglist", ImVec2(300.f, 260.f), ImGuiChildFlags_Borders);
		if (GameplayTagsManager::GetAllTags().empty())
			ImGui::TextDisabled("No tag yet. Tools > Gameplay Tags.");
		if (_allowNone && ImGui::Selectable("(None)"))
		{
			_picked.clear();
			_done = true;
		}

		if (_filter[0])
		{
			const std::string _search = Lower(_filter);
			for (const GameplayTag& _tag : GameplayTagsManager::GetAllTags())
				if (Lower(_tag.GetName()).find(_search) != std::string::npos && ImGui::Selectable(_tag.GetName().c_str()))
				{
					_picked = _tag.GetName();
					_done = true;
				}
		}
		else if (DrawTagTree(GameplayTag(), _picked))
			_done = true;
		ImGui::EndChild();
		return _done;
	}
}

bool Krampus::DrawGameplayTagPicker(const char* _label, std::string& _tag)
{
	bool _changed = false;
	ImGui::PushID(_label);

	const GameplayTag _current = GameplayTag::Request(_tag);
	const bool _bad = !_current.IsNone() && !_current.IsDeclared();
	if (_bad) ImGui::PushStyleColor(ImGuiCol_Text, UNDECLARED);
	if (ImGui::Button((_tag.empty() ? "None" : _tag.c_str()), ImVec2(ImGui::CalcItemWidth(), 0.f)))
		ImGui::OpenPopup("tags");
	if (_bad) ImGui::PopStyleColor();
	if (_bad && ImGui::IsItemHovered()) ImGui::SetTooltip("This tag is not declared in the project (Tools > Gameplay Tags)");
	ImGui::SameLine();
	ImGui::TextUnformatted(_label);

	if (ImGui::BeginPopup("tags"))
	{
		std::string _picked;
		if (DrawPickerBody(_picked, true))
		{
			_tag = _picked;
			_changed = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::PopID();
	return _changed;
}

bool Krampus::DrawGameplayTagContainerEditor(const char* _label, std::string& _encoded)
{
	GameplayTagContainer _tags = GameplayTagContainer::Decode(_encoded);
	bool _changed = false;
	ImGui::PushID(_label);

	ImGui::TextUnformatted(_label);
	ImGui::SameLine();
	if (ImGui::SmallButton("+ Add tag")) ImGui::OpenPopup("tags");
	if (ImGui::BeginPopup("tags"))
	{
		std::string _picked;
		if (DrawPickerBody(_picked, false))
		{
			_changed = _tags.AddTag(GameplayTag::Request(_picked));
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (_tags.IsEmpty()) ImGui::TextDisabled("  no tag");

	// One button per tag, side by side, to the next line when there is no more room. A click removes the tag.
	GameplayTag _remove;
	const float _right = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
	bool _first = true;
	for (const GameplayTag& _tag : _tags)
	{
		const std::string _text = _tag.GetName() + "  x";
		const float _width = ImGui::CalcTextSize(_text.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.f;
		if (!_first)
		{
			if (ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x + _width < _right) ImGui::SameLine();
		}
		_first = false;

		ImGui::PushID(_tag.GetIndex());
		const bool _bad = !_tag.IsDeclared();
		if (_bad) ImGui::PushStyleColor(ImGuiCol_Text, UNDECLARED);
		if (ImGui::SmallButton(_text.c_str())) _remove = _tag;
		if (_bad) ImGui::PopStyleColor();
		if (ImGui::IsItemHovered()) ImGui::SetTooltip(_bad ? "Not declared in the project. Click to remove" : "Click to remove");
		ImGui::PopID();
	}
	if (!_remove.IsNone())
	{
		_tags.RemoveTag(_remove);
		_changed = true;
	}

	ImGui::PopID();
	if (_changed) _encoded = _tags.Encode();
	return _changed;
}

bool Krampus::DrawGameplayTagQueryEditor(const char* _label, std::string& _encoded)
{
	GameplayTagQuery _query = GameplayTagQuery::Decode(_encoded);
	bool _changed = false;

	if (ImGui::TreeNodeEx(_label, ImGuiTreeNodeFlags_DefaultOpen))
	{
		std::string _all = _query.allTags.Encode(), _any = _query.anyTags.Encode(), _none = _query.noneTags.Encode();
		if (DrawGameplayTagContainerEditor("All of", _all)) { _query.allTags = GameplayTagContainer::Decode(_all); _changed = true; }
		if (DrawGameplayTagContainerEditor("Any of", _any)) { _query.anyTags = GameplayTagContainer::Decode(_any); _changed = true; }
		if (DrawGameplayTagContainerEditor("None of", _none)) { _query.noneTags = GameplayTagContainer::Decode(_none); _changed = true; }
		ImGui::TextDisabled("Matches a set of tags that has all of the first, one of the second (if any) and none of the third.");
		ImGui::TreePop();
	}

	if (_changed) _encoded = _query.Encode();
	return _changed;
}

// ---- Tools > Gameplay Tags

namespace
{
	bool		g_open			= false;
	char		g_newName[128]	= "";
	std::string	g_message;
	std::string	g_toRemove;

	// How many actors of the level have this exact tag.
	int UsedBy(const Level* _level, const GameplayTag& _tag)
	{
		int _count = 0;
		if (!_level) return 0;
		for (const std::unique_ptr<Actor>& _actor : const_cast<Level*>(_level)->GetActorManager().GetActors())
			if (const GameplayTagComponent* _component = GameplayTagComponent::Of(_actor.get()))
				if (_component->GetTags().HasTagExact(_tag)) ++_count;
		return _count;
	}

	void SaveTags()
	{
		g_message = GameplayTagsManager::Save() ? "Saved in " + GameplayTagsManager::GetPath() : "Could not write " + GameplayTagsManager::GetPath();
	}

	// One row per tag, children inside. + : a child (fills the name field), x : removes the tag and everything below it.
	void DrawTagRows(const GameplayTag& _parent, const Level* _level)
	{
		for (const GameplayTag& _tag : GameplayTagsManager::GetChildren(_parent))
		{
			const bool _hasChildren = !GameplayTagsManager::GetChildren(_tag).empty();
			ImGui::PushID(_tag.GetIndex());
			ImGuiTreeNodeFlags _flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
			if (!_hasChildren) _flags |= ImGuiTreeNodeFlags_Leaf;
			const bool _open = ImGui::TreeNodeEx(_tag.GetLeafName().c_str(), _flags);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", _tag.GetName().c_str());

			ImGui::SameLine(ImGui::GetContentRegionMax().x - 150.f);
			if (const int _used = UsedBy(_level, _tag)) ImGui::TextDisabled("%d actor%s", _used, _used == 1 ? "" : "s");
			else ImGui::TextDisabled("-");
			ImGui::SameLine(ImGui::GetContentRegionMax().x - 60.f);
			if (ImGui::SmallButton("+"))
			{
				std::snprintf(g_newName, sizeof(g_newName), "%s.", _tag.GetName().c_str());
				ImGui::SetKeyboardFocusHere(-1);
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Add a tag below this one");
			ImGui::SameLine();
			if (ImGui::SmallButton("x")) g_toRemove = _tag.GetName();
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove this tag and the ones below it");

			if (_open)
			{
				if (_hasChildren) DrawTagRows(_tag, _level);
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	}
}

void Krampus::OpenGameplayTagsWindow()
{
	g_open = true;
	ImGui::SetWindowFocus("Gameplay Tags");
}

void Krampus::DrawGameplayTagsWindow(EditorContext& _context)
{
	if (!g_open) return;

	ImGui::SetNextWindowSize(ImVec2(460.f, 560.f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Gameplay Tags", &g_open))
	{
		ImGui::End();
		return;
	}

	const Level* _level = _context.GetLevel();
	ImGui::TextWrapped("The tags of the project, saved in Content/Settings/GameplayTags.cfg. \"A.B.C\" also declares \"A.B\" and \"A\"; a tag matches its parents.");

	// ---- New tag
	ImGui::SetNextItemWidth(260.f);
	const bool _enter = ImGui::InputTextWithHint("##newtag", "New tag, for example Status.Debuff.Burning", g_newName, sizeof(g_newName), ImGuiInputTextFlags_EnterReturnsTrue);
	ImGui::SameLine();
	const std::string _name = g_newName;
	const bool _valid = GameplayTag::IsValidName(_name);
	ImGui::BeginDisabled(!_valid);
	if (ImGui::Button("Add") || (_enter && _valid))
	{
		const GameplayTag _tag = GameplayTagsManager::AddTag(_name);
		if (!_tag.IsNone())
		{
			g_newName[0] = 0;
			SaveTags();
		}
	}
	ImGui::EndDisabled();
	if (!_name.empty() && !_valid)
		ImGui::TextColored(UNDECLARED, "Letters, digits and _ , words separated by a dot (A.B.C)");

	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		GameplayTagsManager::Reload();
		g_message = "Read again from the file";
	}
	if (!g_message.empty()) ImGui::TextDisabled("%s", g_message.c_str());
	ImGui::Separator();

	// ---- The tags
	ImGui::BeginChild("##tagtree", ImVec2(0.f, -ImGui::GetFrameHeightWithSpacing() * 1.f - 90.f), ImGuiChildFlags_Borders);
	if (GameplayTagsManager::GetAllTags().empty())
		ImGui::TextDisabled("No tag yet: type a name above.");
	DrawTagRows(GameplayTag(), _level);
	ImGui::EndChild();

	// ---- Tags of the scene that the project does not declare
	GameplayTagContainer _missing;
	if (_level)
		for (const std::unique_ptr<Actor>& _actor : const_cast<Level*>(_level)->GetActorManager().GetActors())
			if (const GameplayTagComponent* _component = GameplayTagComponent::Of(_actor.get()))
				for (const GameplayTag& _tag : _component->GetTags())
					if (!_tag.IsDeclared()) _missing.AddTag(_tag);
	if (!_missing.IsEmpty())
	{
		ImGui::TextColored(UNDECLARED, "Used in this scene but not declared:");
		for (const GameplayTag& _tag : _missing)
		{
			ImGui::PushID(_tag.GetIndex());
			ImGui::BulletText("%s", _tag.GetName().c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton("Declare"))
			{
				GameplayTagsManager::AddTag(_tag.GetName());
				SaveTags();
			}
			ImGui::PopID();
		}
	}

	// ---- Removing asks first: the scenes keep the tag (shown in red).
	if (!g_toRemove.empty()) ImGui::OpenPopup("Remove tag");
	if (ImGui::BeginPopupModal("Remove tag", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		const GameplayTag _tag = GameplayTag::Request(g_toRemove);
		int _below = 0;
		for (const GameplayTag& _other : GameplayTagsManager::GetAllTags())
			if (_other != _tag && _other.MatchesTag(_tag)) ++_below;
		ImGui::Text("Remove \"%s\"%s?", g_toRemove.c_str(), _below ? (" and the " + std::to_string(_below) + " tag(s) below it").c_str() : "");
		ImGui::TextDisabled("The scenes that use it keep it (shown in red).");
		if (ImGui::Button("Remove"))
		{
			GameplayTagsManager::RemoveTag(_tag);
			SaveTags();
			g_toRemove.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			g_toRemove.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
