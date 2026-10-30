#include "EditorTheme.h"
#include "EditorContext.h"
#include "Utilities/KrampusInclude.h"
#include <imgui-SFML.h>
#include <array>
#include <cstring>
#include <fstream>
#include <sstream>

using namespace Krampus;

namespace
{
	constexpr int CUSTOM_COUNT = static_cast<int>(EditorTheme::Custom::Count);

	// Theme.cfg files without this version were written with the plain ImGui colors: their colors that are still the old
	// defaults are replaced by the Krampus Dark ones, the others (changed by the user) are kept.
	constexpr int THEME_VERSION = 2;

	/// <returns>A color written 0xRRGGBB, as in a paint program.</returns>
	constexpr ImVec4 Rgb(const unsigned int _rgb, const float _alpha = 1.f)
	{
		return ImVec4(((_rgb >> 16) & 0xFF) / 255.f, ((_rgb >> 8) & 0xFF) / 255.f, (_rgb & 0xFF) / 255.f, _alpha);
	}

	constexpr ImVec4 ACCENT = Rgb(0x4C8DF6);

	struct CustomInfo
	{
		const char*		name;
		ImVec4			defaultColor;
		ImVec4			legacyColor;	// its default before THEME_VERSION 2
	};

	const std::array<CustomInfo, CUSTOM_COUNT> customInfos =
	{ {
		{ "Background",			Rgb(0x141517),							ImVec4(0.098f, 0.098f, 0.110f, 1.f) },
		{ "Selected button",	ACCENT,									ImVec4(0.20f, 0.45f, 0.85f, 1.f) },
		{ "Console: log",		ImVec4(0.60f, 0.60f, 0.70f, 1.f),		ImVec4(0.60f, 0.60f, 0.70f, 1.f) },
		{ "Console: message",	ImVec4(1.00f, 1.00f, 1.00f, 1.f),		ImVec4(1.00f, 1.00f, 1.00f, 1.f) },
		{ "Console: warning",	ImVec4(1.00f, 0.85f, 0.20f, 1.f),		ImVec4(1.00f, 0.85f, 0.20f, 1.f) },
		{ "Console: error",		ImVec4(1.00f, 0.35f, 0.35f, 1.f),		ImVec4(1.00f, 0.35f, 0.35f, 1.f) },
		{ "Console: fatal",		ImVec4(1.00f, 0.20f, 0.60f, 1.f),		ImVec4(1.00f, 0.20f, 0.60f, 1.f) },
		{ "Gizmo: X axis",		ImVec4(0.90f, 0.235f, 0.235f, 1.f),		ImVec4(0.90f, 0.235f, 0.235f, 1.f) },
		{ "Gizmo: Y axis",		ImVec4(0.275f, 0.784f, 0.314f, 1.f),	ImVec4(0.275f, 0.784f, 0.314f, 1.f) },
		{ "Gizmo: free move",	ImVec4(0.941f, 0.784f, 0.235f, 1.f),	ImVec4(0.941f, 0.784f, 0.235f, 1.f) },
		{ "Gizmo: rotation",	ImVec4(0.431f, 0.667f, 1.00f, 1.f),		ImVec4(0.431f, 0.667f, 1.00f, 1.f) },
		{ "Play mode",			Rgb(0xE8833A),							Rgb(0xE8833A) },
		{ "Play button",		Rgb(0x3FA35B),							Rgb(0x3FA35B) },
	} };

	std::array<ImVec4, CUSTOM_COUNT> customColors;
	bool customReady = false;
	bool symbolsLoaded = false;	// the font has the Segoe UI Symbol glyphs (play triangle, square...)

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

	/// <returns>True when both colors are the same, up to the rounding of Theme.cfg.</returns>
	bool Same(const ImVec4& _a, const ImVec4& _b)
	{
		constexpr float TOLERANCE = 0.003f;
		return std::abs(_a.x - _b.x) < TOLERANCE && std::abs(_a.y - _b.y) < TOLERANCE && std::abs(_a.z - _b.z) < TOLERANCE && std::abs(_a.w - _b.w) < TOLERANCE;
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
	return EDITOR_DATA_DIRECTORY / "Theme.cfg";
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

void EditorTheme::ApplyKrampusDark()
{
	// Neutral grays with one blue accent: panels a step lighter than the background, tabs and title bars a step darker.
	ImVec4* _c = ImGui::GetStyle().Colors;
	_c[ImGuiCol_Text]						= Rgb(0xE6E8EB);
	_c[ImGuiCol_TextDisabled]				= Rgb(0x7F848E);
	_c[ImGuiCol_WindowBg]					= Rgb(0x1F2023);
	_c[ImGuiCol_ChildBg]					= Rgb(0x000000, 0.f);
	_c[ImGuiCol_PopupBg]					= Rgb(0x26282C, 0.98f);
	_c[ImGuiCol_Border]						= Rgb(0x111214);
	_c[ImGuiCol_BorderShadow]				= Rgb(0x000000, 0.f);
	_c[ImGuiCol_FrameBg]					= Rgb(0x2B2D31);
	_c[ImGuiCol_FrameBgHovered]				= Rgb(0x33363B);
	_c[ImGuiCol_FrameBgActive]				= Rgb(0x3A3E44);
	_c[ImGuiCol_TitleBg]					= Rgb(0x17181A);
	_c[ImGuiCol_TitleBgActive]				= Rgb(0x1B1C1F);
	_c[ImGuiCol_TitleBgCollapsed]			= Rgb(0x17181A);
	_c[ImGuiCol_MenuBarBg]					= Rgb(0x17181A);
	_c[ImGuiCol_ScrollbarBg]				= Rgb(0x000000, 0.f);
	_c[ImGuiCol_ScrollbarGrab]				= Rgb(0x3A3D42);
	_c[ImGuiCol_ScrollbarGrabHovered]		= Rgb(0x474B51);
	_c[ImGuiCol_ScrollbarGrabActive]		= Rgb(0x555A61);
	_c[ImGuiCol_CheckMark]					= ACCENT;
	_c[ImGuiCol_SliderGrab]					= ACCENT;
	_c[ImGuiCol_SliderGrabActive]			= Rgb(0x6FA3F8);
	_c[ImGuiCol_Button]						= Rgb(0x33363B);
	_c[ImGuiCol_ButtonHovered]				= Rgb(0x3E4248);
	_c[ImGuiCol_ButtonActive]				= Rgb(0x3D78D8);
	_c[ImGuiCol_Header]						= Rgb(0x4C8DF6, 0.30f);
	_c[ImGuiCol_HeaderHovered]				= Rgb(0xFFFFFF, 0.07f);
	_c[ImGuiCol_HeaderActive]				= Rgb(0x4C8DF6, 0.45f);
	_c[ImGuiCol_Separator]					= Rgb(0x111214);
	_c[ImGuiCol_SeparatorHovered]			= Rgb(0x4C8DF6, 0.60f);
	_c[ImGuiCol_SeparatorActive]			= ACCENT;
	_c[ImGuiCol_ResizeGrip]					= Rgb(0x000000, 0.f);
	_c[ImGuiCol_ResizeGripHovered]			= Rgb(0x4C8DF6, 0.50f);
	_c[ImGuiCol_ResizeGripActive]			= Rgb(0x4C8DF6, 0.80f);
	_c[ImGuiCol_TabHovered]					= Rgb(0x2E3035);
	_c[ImGuiCol_Tab]						= Rgb(0x17181A);
	_c[ImGuiCol_TabSelected]				= Rgb(0x1F2023);
	_c[ImGuiCol_TabSelectedOverline]		= ACCENT;
	_c[ImGuiCol_TabDimmed]					= Rgb(0x17181A);
	_c[ImGuiCol_TabDimmedSelected]			= Rgb(0x1F2023);
	_c[ImGuiCol_TabDimmedSelectedOverline]	= Rgb(0x3A3D42);
	_c[ImGuiCol_DockingPreview]				= Rgb(0x4C8DF6, 0.55f);
	_c[ImGuiCol_DockingEmptyBg]				= Rgb(0x141517);
	_c[ImGuiCol_PlotLines]					= Rgb(0x9BA0A8);
	_c[ImGuiCol_PlotLinesHovered]			= Rgb(0xF28C5A);
	_c[ImGuiCol_PlotHistogram]				= ACCENT;
	_c[ImGuiCol_PlotHistogramHovered]		= Rgb(0x6FA3F8);
	_c[ImGuiCol_TableHeaderBg]				= Rgb(0x26282C);
	_c[ImGuiCol_TableBorderStrong]			= Rgb(0x111214);
	_c[ImGuiCol_TableBorderLight]			= Rgb(0x2B2D31);
	_c[ImGuiCol_TableRowBg]					= Rgb(0x000000, 0.f);
	_c[ImGuiCol_TableRowBgAlt]				= Rgb(0xFFFFFF, 0.03f);
	_c[ImGuiCol_TextLink]					= ACCENT;
	_c[ImGuiCol_TextSelectedBg]				= Rgb(0x4C8DF6, 0.35f);
	_c[ImGuiCol_DragDropTarget]				= Rgb(0xF2C14E);
	_c[ImGuiCol_NavHighlight]				= ACCENT;
	_c[ImGuiCol_NavWindowingHighlight]		= Rgb(0xFFFFFF, 0.70f);
	_c[ImGuiCol_NavWindowingDimBg]			= Rgb(0x000000, 0.40f);
	_c[ImGuiCol_ModalWindowDimBg]			= Rgb(0x000000, 0.55f);
}

void EditorTheme::ApplyStyle(const float& _scale)
{
	// The sizes start again from the ImGui ones each time: ScaleAllSizes multiplies what is there.
	ImGuiStyle& _style = ImGui::GetStyle();
	ImVec4 _colors[ImGuiCol_COUNT];
	std::memcpy(_colors, _style.Colors, sizeof(_colors));
	_style = ImGuiStyle();
	std::memcpy(_style.Colors, _colors, sizeof(_colors));

	_style.WindowPadding			= ImVec2(10.f, 8.f);
	_style.FramePadding				= ImVec2(8.f, 4.f);
	_style.CellPadding				= ImVec2(6.f, 4.f);
	_style.ItemSpacing				= ImVec2(8.f, 6.f);
	_style.ItemInnerSpacing			= ImVec2(6.f, 4.f);
	_style.IndentSpacing			= 16.f;
	_style.ScrollbarSize			= 12.f;
	_style.GrabMinSize				= 10.f;

	_style.WindowBorderSize			= 1.f;
	_style.ChildBorderSize			= 1.f;
	_style.PopupBorderSize			= 1.f;
	_style.FrameBorderSize			= 0.f;
	_style.TabBorderSize			= 0.f;
	_style.TabBarBorderSize			= 1.f;
	_style.TabBarOverlineSize		= 2.f;
	_style.DockingSeparatorSize		= 2.f;
	_style.SeparatorTextBorderSize	= 2.f;
	_style.SeparatorTextPadding		= ImVec2(16.f, 3.f);

	_style.WindowRounding			= 6.f;
	_style.ChildRounding			= 4.f;
	_style.FrameRounding			= 4.f;
	_style.PopupRounding			= 6.f;
	_style.ScrollbarRounding		= 8.f;
	_style.GrabRounding				= 4.f;
	_style.TabRounding				= 4.f;

	_style.WindowTitleAlign			= ImVec2(0.5f, 0.5f);
	_style.WindowMenuButtonPosition	= ImGuiDir_None;	// no small collapse arrow in the corner of every panel
	_style.ColorButtonPosition		= ImGuiDir_Left;

	_style.ScaleAllSizes(_scale);
}

void EditorTheme::LoadFonts(const float& _scale)
{
	ImGuiIO& _io = ImGui::GetIO();
	_io.Fonts->Clear();
	_io.FontGlobalScale = 1.f;	// the font is made at the right size instead: a scaled one is blurry
	symbolsLoaded = false;

	const float _size = std::round(16.f * _scale);
	const std::filesystem::path _fonts = "C:/Windows/Fonts";
	const std::filesystem::path _text = _fonts / "segoeui.ttf";
	const std::filesystem::path _symbols = _fonts / "seguisym.ttf";

	std::error_code _error;
	ImFont* _font = nullptr;
	if (std::filesystem::exists(_text, _error))	// ImGui stops on an assert when the file is missing: checked first
	{
		ImFontConfig _config;
		_config.OversampleH = 2;
		_config.OversampleV = 1;
		_font = _io.Fonts->AddFontFromFileTTF(_text.string().c_str(), _size, &_config);

		if (_font && std::filesystem::exists(_symbols, _error))
		{
			static const ImWchar SYMBOL_RANGES[] = { 0x2190, 0x21FF, 0x2500, 0x26FF, 0 };	// arrows, shapes, misc symbols
			ImFontConfig _merge;
			_merge.MergeMode = true;
			_merge.OversampleH = 2;
			_merge.OversampleV = 1;
			symbolsLoaded = _io.Fonts->AddFontFromFileTTF(_symbols.string().c_str(), _size, &_merge, SYMBOL_RANGES) != nullptr;
		}
	}
	if (!_font)
	{
		ImFontConfig _config;
		_config.SizePixels = std::round(13.f * _scale);
		_io.Fonts->AddFontDefault(&_config);
		LOG(VerbosityType::Log, "Theme: Segoe UI not found, the ImGui font is used");
	}

	if (!ImGui::SFML::UpdateFontTexture())
		LOG_WARNING("Theme: the font texture of the editor could not be made");
}

const char* EditorTheme::Symbol(const char* _symbol, const char* _fallback)
{
	return symbolsLoaded ? _symbol : _fallback;
}

void EditorTheme::Load()
{
	ApplyKrampusDark();
	ResetCustom();

	std::ifstream _file(GetPath());
	if (!_file.is_open())
	{
		LOG(VerbosityType::Log, "Theme: no Theme.cfg, Krampus Dark colors");
		return;
	}

	std::vector<std::pair<std::string, ImVec4>> _entries;
	int _version = 1;
	std::string _line;
	while (std::getline(_file, _line))
	{
		const size_t _equal = _line.find('=');
		if (_equal == std::string::npos || Trim(_line).rfind('#', 0) == 0) continue;

		const std::string _name = Trim(_line.substr(0, _equal));
		if (_name == "Version")
		{
			_version = std::atoi(Trim(_line.substr(_equal + 1)).c_str());
			continue;
		}
		ImVec4 _value;
		if (Parse(_line.substr(_equal + 1), _value)) _entries.emplace_back(_name, _value);
	}
	_file.close();

	// A file of an older version: its colors that are the old defaults give way to the new theme.
	const bool _old = _version < THEME_VERSION;
	ImGuiStyle _legacy;
	ImGui::StyleColorsDark(&_legacy);

	ImGuiStyle& _style = ImGui::GetStyle();
	int _read = 0, _kept = 0;
	for (const auto& [_name, _value] : _entries)
	{
		bool _found = false;
		for (int _i = 0; _i < ImGuiCol_COUNT && !_found; ++_i)
		{
			if (_name != ImGui::GetStyleColorName(_i)) continue;
			_found = true;
			if (_old && Same(_value, _legacy.Colors[_i])) continue;
			_style.Colors[_i] = _value;
			++_kept;
		}
		for (int _i = 0; _i < CUSTOM_COUNT && !_found; ++_i)
		{
			if (_name != customInfos[_i].name) continue;
			_found = true;
			if (_old && Same(_value, customInfos[_i].legacyColor)) continue;
			customColors[_i] = _value;
			++_kept;
		}
		if (_found) ++_read;
	}

	if (_old)
	{
		Save();
		LOG_MSG("Theme: the editor has its new look (Krampus Dark); " + std::to_string(_kept) + " color(s) you had changed are kept");
	}
	else
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
	_file << "Version = " << THEME_VERSION << "\n";
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

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Presets:");
	ImGui::SameLine();
	if (ImGui::Button("Krampus Dark"))	{ ApplyKrampusDark(); ResetCustom(); _changed = true; }
	ImGui::SameLine();
	if (ImGui::Button("ImGui Dark"))	{ ImGui::StyleColorsDark(); ResetCustom(); customColors[static_cast<int>(Custom::Background)] = customInfos[0].legacyColor; _changed = true; }
	ImGui::SameLine();
	if (ImGui::Button("Light"))			{ ImGui::StyleColorsLight(); ResetCustom(); customColors[static_cast<int>(Custom::Background)] = Rgb(0xC9CBD0); customColors[static_cast<int>(Custom::ConsoleMessage)] = Rgb(0x1A1A1A); _changed = true; }
	ImGui::SameLine();
	if (ImGui::Button("Classic"))		{ ImGui::StyleColorsClassic(); ResetCustom(); _changed = true; }
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Each preset changes every color, the Console and gizmo ones too. Krampus Dark is the default.");

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
			if (!_extrasHeader) { ImGui::SeparatorText("Editor extras (Console, gizmo, Play, background)"); _extrasHeader = true; }
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
