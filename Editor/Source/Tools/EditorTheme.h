#pragma once
#include <imgui.h>
#include <filesystem>

namespace Krampus
{

	/// <summary>
	/// The look of the editor: every color of the Dear ImGui style (windows, buttons, tabs, text...) plus the ones the editor draws itself
	/// (Console lines, gizmo, Viewport background), the sizes (rounding, padding) and the font.
	/// Colors are edited in Settings > Colors and stored in Content/_internal/Editor/Theme.cfg.
	/// </summary>
	namespace EditorTheme
	{
		enum class Custom
		{
			Background,		// behind the docked windows
			Selection,		// selected tool button
			ConsoleLog,
			ConsoleMessage,
			ConsoleWarning,
			ConsoleError,
			ConsoleFatal,
			GizmoX,
			GizmoY,
			GizmoFree,
			GizmoRing,
			PlayMode,		// menu bar, Stop button and Viewport border while the game runs
			PlayButton,		// the Play button
			Count
		};

		std::filesystem::path GetPath();

		/// <summary>Reads Theme.cfg (missing: the Krampus Dark theme) and applies it to ImGui. Needs an ImGui context.</summary>
		void Load();

		/// <returns>False if the file could not be written.</returns>
		bool Save();

		/// <summary>The colors of the Krampus Dark theme (the default one).</summary>
		void ApplyKrampusDark();

		/// <summary>
		/// Sizes of the interface (rounding, padding, spacing) for this interface scale. Called again when Settings > Editor > uiScale changes.
		/// </summary>
		void ApplyStyle(const float& _scale);

		/// <summary>
		/// Loads the font of the editor (Segoe UI, with the symbols of Segoe UI Symbol) at this interface scale: sharp at any size,
		/// unlike a scaled bitmap font. Falls back on the ImGui font when Windows has none. Call it between two frames.
		/// </summary>
		void LoadFonts(const float& _scale);

		/// <returns>_symbol (UTF-8, such as a play triangle) when the font has the symbols, else _fallback.</returns>
		const char* Symbol(const char* _symbol, const char* _fallback);

		/// <returns>A color the editor draws itself.</returns>
		ImVec4 Get(const Custom& _color);
		ImU32 GetU32(const Custom& _color);

		/// <returns>The color lightened, for an element under the mouse.</returns>
		ImU32 Lighten(const ImU32& _color);

		/// <returns>The color darkened.</returns>
		ImU32 Darken(const ImU32& _color);

		/// <summary>The Colors page of the Settings window. Returns true when a color changed.</summary>
		bool DrawSettingsPage();
	}
}
