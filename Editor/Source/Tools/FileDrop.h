#pragma once
#include <imgui.h>
#include <filesystem>
#include <vector>

namespace Krampus
{

	/// <summary>
	/// Files and folders dropped on the editor window from the Windows Explorer (SFML does not see them: the window is told by
	/// Windows with WM_DROPFILES). The Content panel takes them and copies them into the folder it shows.
	/// </summary>
	namespace FileDrop
	{
		struct Drop
		{
			std::vector<std::filesystem::path>	paths;
			ImVec2								position;	// where the mouse let go, in window pixels (the ImGui coordinates)
		};

		/// <summary>Lets the window take dropped files. Call it once the window exists.</summary>
		void Install(void* _windowHandle);

		/// <summary>Stops it. Must be called before the editor module is unloaded (hot reload): Windows would call code that is gone.</summary>
		void Uninstall();

		/// <returns>True and the oldest drop not taken yet, removed from the waiting ones.</returns>
		bool Take(Drop& _drop);
	}
}
