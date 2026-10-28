#pragma once
#include "Core/Layer.h"
#include "Utilities/Debug/Logger.h"
#include "EditorContext.h"

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
		Bool OnEvent(const sf::Event& _event) override;
		void OnRender(Window& _window) override;

	private:
		void BuildDefaultLayout(unsigned int _dockspace);
		void DrawViewport();
		void DrawConsole();
	};
}
