#include "Shortcuts.h"
#include <imgui_internal.h>

using namespace Krampus;

namespace
{
	const std::filesystem::path FILE_PATH = EDITOR_DATA_DIRECTORY / "Shortcuts.cfg";

	std::string Lower(std::string _text)
	{
		std::transform(_text.begin(), _text.end(), _text.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
		return _text;
	}

	// Keys that make a chord on their own: modifiers, mouse and gamepad are left out.
	bool IsBindableKey(const ImGuiKey& _key)
	{
		if (_key >= ImGuiKey_LeftCtrl && _key <= ImGuiKey_RightSuper) return false;
		if (_key == ImGuiKey_ReservedForModCtrl || _key == ImGuiKey_ReservedForModShift || _key == ImGuiKey_ReservedForModAlt || _key == ImGuiKey_ReservedForModSuper) return false;
		if (ImGui::IsMouseKey(_key) || ImGui::IsGamepadKey(_key)) return false;
		return true;
	}

	bool Conflicts(const Shortcuts::Action& _a, const Shortcuts::Action& _b)
	{
		if (&_a == &_b || !_a.chord.IsBound() || !(_a.chord == _b.chord)) return false;
		return _a.context == _b.context || _a.context == "Global" || _b.context == "Global";
	}

	std::string capturing;	// action waiting for its new key
}

std::string KeyChord::ToString() const
{
	if (!IsBound()) return "";
	std::string _text;
	if (ctrl) _text += "Ctrl+";
	if (shift) _text += "Shift+";
	if (alt) _text += "Alt+";
	return _text + ImGui::GetKeyName(static_cast<ImGuiKey>(key));
}

KeyChord KeyChord::Parse(const std::string& _text)
{
	KeyChord _chord;
	std::stringstream _stream(_text);
	std::string _part;
	while (std::getline(_stream, _part, '+'))
	{
		const size_t _begin = _part.find_first_not_of(' ');
		if (_begin == std::string::npos) continue;
		_part = _part.substr(_begin, _part.find_last_not_of(' ') - _begin + 1);

		const std::string _lower = Lower(_part);
		if (_lower == "ctrl") { _chord.ctrl = true; continue; }
		if (_lower == "shift") { _chord.shift = true; continue; }
		if (_lower == "alt") { _chord.alt = true; continue; }

		for (int _k = ImGuiKey_NamedKey_BEGIN; _k < ImGuiKey_NamedKey_END; ++_k)
			if (IsBindableKey(static_cast<ImGuiKey>(_k)) && Lower(ImGui::GetKeyName(static_cast<ImGuiKey>(_k))) == _lower)
				_chord.key = _k;
	}
	return _chord;
}

std::vector<Shortcuts::Action>& Shortcuts::Actions()
{
	static std::vector<Action> _actions;
	return _actions;
}

void Shortcuts::Initialize()
{
	static bool _done = false;
	if (_done) return;
	_done = true;

	const auto _add = [](const char* _id, const char* _context, const KeyChord& _chord, const bool& _allowInText = false)
	{
		Action _action;
		_action.id = _id;
		_action.context = _context;
		_action.defaults = _chord;
		_action.chord = _chord;
		_action.allowInText = _allowInText;
		Actions().push_back(_action);
	};

	// { key, ctrl, shift, alt }
	_add("Save Scene",			"Global",	{ ImGuiKey_S, true, false, false }, true);
	_add("Undo",				"Global",	{ ImGuiKey_Z, true, false, false });
	_add("Redo",				"Global",	{ ImGuiKey_Y, true, false, false });
	_add("Redo (second key)",	"Global",	{ ImGuiKey_Z, true, true, false });
	_add("Play / Stop",			"Global",	{ ImGuiKey_P, true, false, false });
	_add("Compile",				"Global",	{ ImGuiKey_F7, false, false, false }, true);
	_add("Open Solution",		"Global",	{ });
	_add("Package Game",		"Global",	{ });
	_add("Gizmo: All tools",	"Viewport",	{ ImGuiKey_Q, false, false, false });
	_add("Gizmo: Move",			"Viewport",	{ ImGuiKey_W, false, false, false });
	_add("Gizmo: Rotate",		"Viewport",	{ ImGuiKey_E, false, false, false });
	_add("Gizmo: Scale",		"Viewport",	{ ImGuiKey_R, false, false, false });
	_add("Focus Selection",		"Viewport",	{ ImGuiKey_F, false, false, false });
	_add("Delete Selection",	"Scene",	{ ImGuiKey_Delete, false, false, false });
	_add("Rename File",			"Content",	{ ImGuiKey_F2, false, false, false });
	_add("Delete File",			"Content",	{ ImGuiKey_Delete, false, false, false });

	// The file only holds what was changed.
	std::ifstream _file(FILE_PATH);
	std::string _line;
	while (std::getline(_file, _line))
	{
		const size_t _equal = _line.find('=');
		if (_line.empty() || _line[0] == '#' || _equal == std::string::npos) continue;

		std::string _id = _line.substr(0, _equal);
		while (!_id.empty() && _id.back() == ' ') _id.pop_back();
		std::string _value = _line.substr(_equal + 1);
		if (!_value.empty() && _value.back() == '\r') _value.pop_back();

		for (Action& _action : Actions())
			if (_action.id == _id) _action.chord = KeyChord::Parse(_value);
	}
}

bool Shortcuts::Pressed(const std::string& _id)
{
	for (const Action& _action : Actions())
	{
		if (_action.id != _id) continue;
		if (!_action.chord.IsBound() || !capturing.empty()) return false;

		const ImGuiIO& _io = ImGui::GetIO();
		if (_io.WantTextInput && !_action.allowInText) return false;
		if (_io.KeyCtrl != _action.chord.ctrl || _io.KeyShift != _action.chord.shift || _io.KeyAlt != _action.chord.alt) return false;
		return ImGui::IsKeyPressed(static_cast<ImGuiKey>(_action.chord.key), false);
	}
	return false;
}

std::string Shortcuts::Text(const std::string& _id)
{
	for (const Action& _action : Actions())
		if (_action.id == _id) return _action.chord.ToString();
	return "";
}

bool Shortcuts::Save()
{
	std::error_code _error;
	std::filesystem::create_directories(FILE_PATH.parent_path(), _error);

	std::ofstream _file(FILE_PATH, std::ios::trunc);
	if (!_file) return false;

	_file << "# Keyboard shortcuts of the editor. Edited in Settings > Shortcuts. Only the keys that differ from the default are listed.\n";
	for (const Action& _action : Actions())
		if (!(_action.chord == _action.defaults))
			_file << _action.id << " = " << _action.chord.ToString() << "\n";
	return _file.good();
}

void Shortcuts::DrawSettingsPage()
{
	ImGui::SeparatorText("Shortcuts");
	ImGui::TextWrapped("Click a key, then press the new one (with Ctrl, Shift or Alt if you want). Esc cancels. "
		"Two actions of the same area cannot share a key: the older one is unbound.");
	ImGui::TextDisabled("F5 (reload the game) belongs to the editor window itself and cannot be changed here.");
	ImGui::Spacing();

	bool _changed = false;

	// What was pressed while an action waits for its key.
	KeyChord _captured;
	bool _hasCaptured = false;
	if (!capturing.empty())
	{
		const ImGuiIO& _io = ImGui::GetIO();
		if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
		{
			capturing.clear();
		}
		else
		{
			for (int _k = ImGuiKey_NamedKey_BEGIN; _k < ImGuiKey_NamedKey_END && !_hasCaptured; ++_k)
			{
				const ImGuiKey _key = static_cast<ImGuiKey>(_k);
				if (!IsBindableKey(_key) || !ImGui::IsKeyPressed(_key, false)) continue;
				_captured = { _k, _io.KeyCtrl, _io.KeyShift, _io.KeyAlt };
				_hasCaptured = true;
			}
		}
	}

	if (ImGui::BeginTable("##shortcuts", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch, 2.f);
		ImGui::TableSetupColumn("Area", ImGuiTableColumnFlags_WidthStretch, 1.f);
		ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 1.5f);
		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 110.f);
		ImGui::TableHeadersRow();

		for (Action& _action : Actions())
		{
			ImGui::PushID(_action.id.c_str());
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(_action.id.c_str());
			ImGui::TableNextColumn();
			ImGui::TextDisabled("%s", _action.context.c_str());

			ImGui::TableNextColumn();
			const bool _waiting = capturing == _action.id;
			const std::string _label = _waiting ? "Press the new key..." : (_action.chord.IsBound() ? _action.chord.ToString() : "(none)");
			if (_waiting) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.55f, 0.15f, 1.f));
			if (ImGui::Button(_label.c_str(), ImVec2(-FLT_MIN, 0.f)))
				capturing = _waiting ? "" : _action.id;
			if (_waiting) ImGui::PopStyleColor();

			// A conflict is shown while the shortcut is in use.
			for (const Action& _other : Actions())
				if (Conflicts(_action, _other))
					ImGui::TextColored(ImVec4(1.f, 0.5f, 0.3f, 1.f), "same key as %s", _other.id.c_str());

			ImGui::TableNextColumn();
			if (ImGui::SmallButton("Reset"))
			{
				_action.chord = _action.defaults;
				_changed = true;
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("Clear"))
			{
				_action.chord = KeyChord();
				_changed = true;
			}

			if (_waiting && _hasCaptured)
			{
				_action.chord = _captured;
				capturing.clear();
				_changed = true;

				// Whoever had this key in the same area loses it.
				for (Action& _other : Actions())
					if (Conflicts(_action, _other))
					{
						LOG_WARNING("Shortcuts: " + _other.id + " lost " + _other.chord.ToString() + " (now used by " + _action.id + ")");
						_other.chord = KeyChord();
					}
				LOG_MSG("Shortcuts: " + _action.id + " = " + _action.chord.ToString());
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
	}

	ImGui::Spacing();
	if (ImGui::Button("Reset all to defaults"))
	{
		for (Action& _action : Actions()) _action.chord = _action.defaults;
		capturing.clear();
		_changed = true;
	}

	if (_changed && !Save())
		LOG_ERROR("Shortcuts: cannot write " + FILE_PATH.generic_string());
}
