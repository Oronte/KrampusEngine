#pragma once
#include "Core/Layer.h"
#include "Utilities/Debug/Logger.h"
#include "EditorContext.h"
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
		static constexpr size_t		MAX_CONSOLE_LINES	= 2000;

		Window&						window;
		EditorContext				context;
		sf::Clock					clock;
		sf::Texture					viewportTexture;
		std::string					iniPath				= "Content/Editor/imgui.ini";
		bool						initialized			= false;
		bool						viewportHovered		= false;
		bool						layoutChecked		= false;

		int							gizmoAxis			= -1;		// handle being dragged: 0 X arrow, 1 Y arrow, 2 center, -1 none
		ImVec2						gizmoStartMouse;
		FVector2					gizmoStartPosition;

		std::vector<Logger::Entry>	consoleLines;
		bool						autoScroll			= true;

	public:
		EditorLayer(Engine* _engine)
			: window(_engine->GetWindowRef())
		{
			context.engine = _engine;
		}

		void OnAttach() override;
		void OnDetach() override;
		Bool BlocksInput() const override { return !context.playing; } // outside Play mode the game gets no input
		/// <summary>After a hot reload: opens again the scene that was being edited. Call it once the game chose its first scene.</summary>
		void RestoreSession() { Krampus::RestoreSession(context); }

		Bool OnEvent(const sf::Event& _event) override;
		void OnRender(Window& _window) override;

	private:
		void BuildDefaultLayout(unsigned int _dockspace);
		void DrawViewport();
		void DrawGizmo(const ImVec2& _min, const ImVec2& _max);
		void DrawConsole();
	};
}
