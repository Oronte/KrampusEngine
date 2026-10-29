#pragma once
#include <imgui.h>
#include <filesystem>

namespace Krampus
{

	/// <summary>
	/// The colors of the editor: every color of the Dear ImGui style (windows, buttons, tabs, text...) plus the ones the editor draws itself
	/// (Console lines, gizmo, Viewport background). Edited in Settings > Colors, stored in Content/Editor/Theme.cfg.
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
			Count
		};

		std::filesystem::path GetPath();

		/// <summary>Reads Theme.cfg (missing: the default dark theme) and applies it to ImGui. Needs an ImGui context.</summary>
		void Load();

		/// <returns>False if the file could not be written.</returns>
		bool Save();

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
