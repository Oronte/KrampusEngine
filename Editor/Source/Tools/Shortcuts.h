#pragma once
#include "EditorContext.h"
#include <imgui.h>

namespace Krampus
{

	/// <summary>
	/// A key and the modifiers that must be held with it ("Ctrl+Shift+Z").
	/// </summary>
	struct KeyChord
	{
		int		key		= 0;		// an ImGuiKey, ImGuiKey_None (0) = not bound
		bool	ctrl	= false;
		bool	shift	= false;
		bool	alt		= false;

		bool IsBound() const { return key != 0; }
		bool operator==(const KeyChord& _other) const { return key == _other.key && ctrl == _other.ctrl && shift == _other.shift && alt == _other.alt; }

		/// <returns>"Ctrl+Shift+Z", or an empty text when not bound.</returns>
		std::string ToString() const;

		/// <returns>The chord written by ToString (not bound if the text is not understood).</returns>
		static KeyChord Parse(const std::string& _text);
	};

	/// <summary>
	/// Keyboard shortcuts of the editor: a list of named actions, each with a key that the user can change in
	/// Settings > Shortcuts. The bindings are stored in Content/_internal/Editor/Shortcuts.cfg.
	/// Code asks "is this action pressed now?" with the name of the action, never with a key.
	/// </summary>
	class Shortcuts
	{
	public:
		struct Action
		{
			std::string		id;					// what the code asks for, and the name shown
			std::string		context;			// "Global", "Viewport", "Scene" or "Content": two actions only conflict in the same context
			KeyChord		defaults;
			KeyChord		chord;
			bool			allowInText	= false;	// still works while a text field is being edited
		};

		/// <summary>Registers the actions of the editor and reads the file. Call once.</summary>
		static void Initialize();

		/// <returns>True on the frame the shortcut of the action is pressed.</returns>
		static bool Pressed(const std::string& _id);

		/// <returns>"Ctrl+S" for a menu item, empty if the action is not bound.</returns>
		static std::string Text(const std::string& _id);

		static bool Save();

		/// <summary>The page of the Settings window: every action with its key, click one then press the new keys.</summary>
		static void DrawSettingsPage();

		static std::vector<Action>& Actions();
	};
}
