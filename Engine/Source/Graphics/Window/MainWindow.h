#pragma once
#include "Window.h"
#include "Utilities/System/Singleton.h"

#define MAIN_WINDOW Krampus::MainWindow::GetInstance()

namespace Krampus
{

	class MainWindow : public Window, public Singleton<MainWindow>
	{
	public:
		MainWindow() = default;

		void RenderGui();
	};

}