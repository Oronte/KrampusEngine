#pragma once
#include "Core/Layer.h"
#include "Utilities/Debug/Logger.h"
#include "EditorContext.h"
#include "EditorSettings.h"
#include <imgui.h>

namespace Krampus
{

	/// <summary>
	/// ImGui overlay: main menu, dockable Viewport (the game picture), Hierarchy, Inspector, Content browser and Console (the Logger output).
	/// Push it above the layers that must appear in the Viewport.
	/// Known limit: the game still renders at window size and is only scaled into the Viewport.
	/// </summary>
	class EditorLayer : public Layer
	{

		Window&						window;
		EditorContext				context;
		sf::Clock					clock;
		sf::Texture					viewportTexture;
		std::string					iniPath				= (EDITOR_DATA_DIRECTORY / "imgui.ini").generic_string();
		bool						initialized			= false;
		bool						viewportHovered		= false;
		bool						cameraDragging		= false;	// right or middle mouse held: the editor camera is being moved
		bool						layoutChecked		= false;
		float						appliedUiScale		= 0.f;		// interface scale the font and sizes were made for

		int							gizmoAxis			= -1;		// handle being dragged: 0 X arrow, 1 Y arrow, 2 center, 3 rotation ring, 4 X box, 5 Y box, 6 grey box, -1 none
		int							gizmoMode			= 0;		// 0 all tools, 1 move, 2 rotate, 3 scale
		float						gizmoStartRotation	= 0.f;		// degrees
		FVector2					gizmoStartScale;
		float						gizmoLastAngle		= 0.f;		// radians, mouse around the center
		float						gizmoAccum			= 0.f;		// radians turned since the drag started
		ImVec2						gizmoStartMouse;
		FVector2					gizmoStartPosition;

		std::vector<Logger::Entry>	consoleLines;
		ImGuiTextFilter				consoleSearch;					// "word", "a,b" (one of them), "-word" (not this one)
		std::string					consoleCategory;				// empty: every category; else only the lines "Category: ..."
		std::vector<int>			consoleVisible;					// indices in consoleLines that pass the filters (rebuilt each frame)
		EditorSettings				editorSettings;		// preferences of the editor (Settings window)
		bool						settingsDirty		= false;	// a setting was changed and not saved yet

	public:
		EditorLayer(Engine* _engine)
			: window(_engine->GetWindowRef())
		{
			context.engine = _engine;
		}

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(const Float& _deltaTime) override;
		Bool BlocksInput() const override { return !context.playing; } // outside Play mode the game gets no input
		/// <summary>After a hot reload: opens again the scene that was being edited. Call it once the game chose its first scene.</summary>
		void RestoreSession() { Krampus::RestoreSession(context); }

		Bool OnEvent(const sf::Event& _event) override;
		void OnRender(Window& _window) override;

	private:
		void BuildDefaultLayout(unsigned int _dockspace);
		void DrawViewport();
		void DrawGizmo(const ImVec2& _min, const ImVec2& _max);
		void DrawViewportOverlay(const ImVec2& _min, const ImVec2& _max);
		void PickInViewport(const ImVec2& _min, const ImVec2& _max);
		void MoveCamera(const ImVec2& _min, const ImVec2& _max);
		void DrawSettingsWindow();
		void HandleShortcuts();
		void SaveSettings();
		void DrawConsole();
	};
}
