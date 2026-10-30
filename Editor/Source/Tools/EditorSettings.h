#pragma once
#include "EditorContext.h"
#include "Reflection/Reflection.h"

namespace Krampus
{

	/// <summary>
	/// Preferences of the editor itself (not of the game). Stored in Content/_internal/Editor/EditorSettings.cfg and edited in the Settings window,
	/// the same way as the engine settings.
	/// </summary>
	class EditorSettings : public SettingsSection
	{
		KCLASS(EditorSettings, IGameObject)

	public:
		const char* Title() const override { return "Editor"; }
		const char* Description() const override { return "Look and behavior of the editor: interface size, Console, move arrows."; }

		KPROPERTY_RANGE(Float, uiScale, 0.7f, 2.0f) = 1.0f;			// size of the interface
		KPROPERTY(Bool, showGizmo) = true;							// move arrows on the selection
		KPROPERTY_RANGE(Float, gizmoSize, 40.f, 140.f) = 70.f;		// length of the arrows in pixels
		KPROPERTY(Bool, showBillboards) = true;						// icons of the BillboardComponents in the Viewport
		KPROPERTY_RANGE(Int, consoleMaxLines, 100, 20000) = 2000;	// lines kept by the Console
		KPROPERTY(Bool, autoScrollConsole) = true;

		// Console filters (the check boxes at the top of the Console), kept from one session to the next.
		KPROPERTY(Bool, consoleShowVerbose) = true;		// Verbose and VeryVerbose
		KPROPERTY(Bool, consoleShowLog) = true;
		KPROPERTY(Bool, consoleShowMessages) = true;	// Display
		KPROPERTY(Bool, consoleShowWarnings) = true;
		KPROPERTY(Bool, consoleShowErrors) = true;		// Error and Fatal
		KPROPERTY(Bool, consoleShowSource) = false;		// the "(File : ... | Func : ... | Line : ...)" end of each line

		void ResetToDefaults() override
		{
			uiScale = 1.0f;
			showGizmo = true;
			gizmoSize = 70.f;
			showBillboards = true;
			consoleMaxLines = 2000;
			autoScrollConsole = true;
			consoleShowVerbose = true;
			consoleShowLog = true;
			consoleShowMessages = true;
			consoleShowWarnings = true;
			consoleShowErrors = true;
			consoleShowSource = false;
		}

		static std::filesystem::path GetPath()
		{
			return EDITOR_DATA_DIRECTORY / "EditorSettings.cfg";
		}
	};
}
