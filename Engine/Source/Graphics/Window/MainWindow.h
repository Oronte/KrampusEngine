#pragma once
#include "Window.h"

#define MAIN_WINDOW Krampus::MainWindow::GetInstance()

namespace Krampus
{

	class MainWindow : public Window, public Singleton<MainWindow>
	{
	public:
		MainWindow() = default;

		INLINE void RenderGui()
		{
			//ImGui::SFML::Render(GetRenderWindow());
		}
	};

}