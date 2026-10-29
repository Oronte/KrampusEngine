#include "EditorTheme.h"
#include "Utilities/KrampusInclude.h"
#include <array>
#include <fstream>
#include <sstream>

using namespace Krampus;

namespace
{
	constexpr int CUSTOM_COUNT = static_cast<int>(EditorTheme::Custom::Count);

	struct CustomInfo
	{
		const char*		name;
		ImVec4			defaultColor;
	};

	const std::array<CustomInfo, CUSTOM_COUNT> customInfos =
	{ {
		{ "Background",			ImVec4(0.098f, 0.098f, 0.110f, 1.f) },
		{ "Selected button",	ImVec4(0.20f, 0.45f, 0.85f, 1.f) },
		{ "Console: log",		ImVec4(0.60f, 0.60f, 0.70f, 1.f) },
		{ "Console: message",	ImVec4(1.00f, 1.00f, 1.00f, 1.f) },
		{ "Console: warning",	ImVec4(1.00f, 0.85f, 0.20f, 1.f) },
		{ "Console: error",		ImVec4(1.00f, 0.35f, 0.35f, 1.f) },
		{ "Console: fatal",		ImVec4(1.00f, 0.20f, 0.60f, 1.f) },
		{ "Gizmo: X axis",		ImVec4(0.90f, 0.235f, 0.235f, 1.f) },
		{ "Gizmo: Y axis",		ImVec4(0.275f, 0.784f, 0.314f, 1.f) },
		{ "Gizmo: free move",	ImVec4(0.941f, 0.784f, 0.235f, 1.f) },
		{ "Gizmo: rotation",	ImVec4(0.431f, 0.667f, 1.00f, 1.f) },
	} };

	std::array<ImVec4, CUSTOM_COUNT> customColors;
	bool customReady = false;

	void EnsureCustom()
	{
		if (customReady) return;
		for (int _i = 0; _i < CUSTOM_COUNT; ++_i)
			customColors[_i] = customInfos[_i].defaultColor;
		customReady = true;
	}

	void ResetCustom()
	{
		customReady = false;
		EnsureCustom();
	}

	std::string Format(const ImVec4& _c)
	{
		std::ostringstream _out;
		_out << _c.x << ", " << _c.y << ", " << _c.z << ", " << _c.w;
		return _out.str();
	}

	bool Parse(const std::string& _text, ImVec4& _out)
	{
		std::string _clean = _text;
		std::replace(_clean.begin(), _clean.end(), ',', ' ');
		std::istringstream _in(_clean);
		ImVec4 _c;
		if (!(_in >> _c.x >> _c.y >> _c.z >> _c.w)) return false;
		_out = _c;
		return true;
	}

	std::string Trim(const std::string& _s)
	{
		const size_t _a = _s.find_first_not_of(" \t\r");
		if (_a == std::string::npos) return "";
		return _s.substr(_a, _s.find_last_not_of(" \t\r") - _a + 1);
	}

	ImU32 Scale(const ImU32& _color, const float& _factor, const float& _add)
	{
		ImVec4 _c = ImGui::ColorConvertU32ToFloat4(_color);
		_c.x = std::clamp(_c.x * _factor + _add, 0.f, 1.f);
		_c.y = std::clamp(_c.y * _factor + _add, 0.f, 1.f);
		_c.z = std::clamp(_c.z * _factor + _add, 0.f, 1.f);
		return ImGui::ColorConvertFloat4ToU32(_c);
	}
}

std::filesystem::path EditorTheme::GetPath()
{
	return std::filesystem::path("Content/Editor/Theme.cfg");
}

ImVec4 EditorTheme::Get(const Custom& _color)
{
	EnsureCustom();
	return customColors[static_cast<int>(_color)];
}

ImU32 EditorTheme::GetU32(const Custom& _color)
{
	return ImGui::ColorConvertFloat4ToU32(Get(_color));
}

ImU32 EditorTheme::Lighten(const ImU32& _color)
{
	return Scale(_color, 0.5f, 0.5f);
}

ImU32 EditorTheme::Darken(const ImU32& _color)
{
	return Scale(_color, 0.8f, 0.f);
}

void EditorTheme::Load()
{
	EnsureCustom();
	ImGui::StyleColorsDark();
	ResetCustom();

	std::ifstream _file(GetPath());
	if (!_file.is_open())
	{
		LOG(VerbosityType::Log, "Theme: no Theme.cfg, default colors");
		return;
	}

	ImGuiStyle& _style = ImGui::GetStyle();
	std::string _line;
	int _read = 0;
	while (std::getline(_file, _line))
	{
		const size_t _equal = _line.find('=');
		if (_equal == std::string::npos) continue;

		const std::string _name = Trim(_line.substr(0, _equal));
		ImVec4 _value;
		if (!Parse(_line.substr(_equal + 1), _value)) continue;

		bool _found = false;
		for (int _i = 0; _i < ImGuiCol_COUNT && !_found; ++_i)
			if (_name == ImGui::GetStyleColorName(_i)) { _style.Colors[_i] = _value; _found = true; }
		for (int _i = 0; _i < CUSTOM_COUNT && !_found; ++_i)
			if (_name == customInfos[_i].name) { customColors[_i] = _value; _found = true; }
		if (_found) ++_read;
	}
	LOG(VerbosityType::Log, "Theme: loaded " + std::to_string(_read) + " colors");
}

bool EditorTheme::Save()
{
	EnsureCustom();
	std::error_code _error;
	std::filesystem::create_directories(GetPath().parent_path(), _error);

	std::ofstream _file(GetPath(), std::ios::trunc);
	if (!_file.is_open()) return false;

	_file << "# Colors of the editor, edited in Settings > Colors. Format: name = r, g, b, a (0 to 1).\n";
	const ImGuiStyle& _style = ImGui::GetStyle();
	for (int _i = 0; _i < ImGuiCol_COUNT; ++_i)
		_file << ImGui::GetStyleColorName(_i) << " = " << Format(_style.Colors[_i]) << "\n";
	for (int _i = 0; _i < CUSTOM_COUNT; ++_i)
		_file << customInfos[_i].name << " = " << Format(customColors[_i]) << "\n";
	return _file.good();
}

bool EditorTheme::DrawSettingsPage()
{
	EnsureCustom();
	bool _changed = false;
	ImGuiStyle& _style = ImGui::GetStyle();

	ImGui::SeparatorText("Colors");
	ImGui::TextWrapped("Every color of the editor. A change shows at once; it is saved with the other settings (Theme.cfg).");
	ImGui::Spacing();

	ImGui::Text("Presets:");
	ImGui::SameLine();
	if (ImGui::Button("Dark"))		{ ImGui::StyleColorsDark();		_changed = true; }
	ImGui::SameLine();
	if (ImGui::Button("Light"))		{ ImGui::StyleColorsLight();	_changed = true; }
	ImGui::SameLine();
	if (ImGui::Button("Classic"))	{ ImGui::StyleColorsClassic();	_changed = true; }
	ImGui::SameLine();
	if (ImGui::Button("Reset all"))	{ ImGui::StyleColorsDark(); ResetCustom(); _changed = true; }

	static char filter[64] = "";
	ImGui::SetNextItemWidth(220.f);
	ImGui::InputTextWithHint("##colorFilter", "Search a color...", filter, sizeof(filter));
	const std::string _needle = [&]()
	{
		std::string _s = filter;
		std::transform(_s.begin(), _s.end(), _s.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
		return _s;
	}();
	const auto _matches = [&](const char* _name)
	{
		if (_needle.empty()) return true;
		std::string _s = _name;
		std::transform(_s.begin(), _s.end(), _s.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
		return _s.find(_needle) != std::string::npos;
	};

	const ImGuiColorEditFlags _flags = ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf;
	if (ImGui::BeginChild("##colors", ImVec2(0.f, 0.f)))
	{
		bool _extrasHeader = false;
		for (int _i = 0; _i < CUSTOM_COUNT; ++_i)
		{
			if (!_matches(customInfos[_i].name)) continue;
			if (!_extrasHeader) { ImGui::SeparatorText("Editor extras (Console, gizmo, background)"); _extrasHeader = true; }
			ImGui::PushID(_i);
			if (ImGui::ColorEdit4(customInfos[_i].name, &customColors[_i].x, _flags)) _changed = true;
			ImGui::PopID();
		}

		bool _header = false;
		for (int _i = 0; _i < ImGuiCol_COUNT; ++_i)
		{
			const char* _name = ImGui::GetStyleColorName(_i);
			if (!_matches(_name)) continue;
			if (!_header) { ImGui::SeparatorText("Interface"); _header = true; }
			ImGui::PushID(1000 + _i);
			if (ImGui::ColorEdit4(_name, &_style.Colors[_i].x, _flags)) _changed = true;
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	return _changed;
}
