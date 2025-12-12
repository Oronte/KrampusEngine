#include "MainWindow.h"
//#include "ImGui/imgui.h"
//#include "ImGui/imgui-SFML.h"
#include "Graphics/GuiDrawer.h"

void Krampus::MainWindow::RenderGui()
{
	GuiDrawer::Draw();
	//ImGui::SFML::Render(GetRenderWindow());
}
