#include "GuiDrawer.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui-SFML.h"
#include "Core/Engine.h"
#include "Graphics/Window/MainWindow.h"

#include <Windows.h>

void Krampus::GuiDrawer::Draw()
{
    sf::RenderWindow& window = MAIN_WINDOW.GetRenderWindow();
    sf::Vector2u winSize = window.getSize();
    sf::Vector2i winPos = window.getPosition(); // Position de la fenêtre

    // --- Barre de titre ---
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)winSize.x, 30.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##titlebar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    // Nom
    ImGui::Text("Krampus Engine");
    ImGui::SameLine(winSize.x - 90);

    // Boutons
    if (ImGui::Button("_")) // Minimize
    {
#ifdef _WIN32
        ShowWindow((HWND)window.getNativeHandle(), SW_MINIMIZE);
#endif
    }
    ImGui::SameLine();
    if (ImGui::Button("[ ]")) // Max / Restore
    {
        sf::Vector2u currentSize = window.getSize();
        if (currentSize.x < 1920 || currentSize.y < 1080)
        {
            window.setSize(sf::Vector2u(1920, 1080));
            window.setPosition(sf::Vector2i(0, 0));
        }
        else
        {
            window.setSize(sf::Vector2u(800, 600));
            window.setPosition(sf::Vector2i(50, 50));
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("X")) // Close
    {
        ENGINE.onWindowClosed.Broadcast();
    }

    // Drag de la fenêtre
    ImGui::InvisibleButton("title_drag", ImVec2((float)winSize.x, 30));
    if (ImGui::IsItemActive())
    {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        winPos.x += (int)delta.x;
        winPos.y += (int)delta.y;
        window.setPosition(winPos);
    }

    ImGui::End();
    ImGui::PopStyleVar(2);

    // --- Poignées de resize (coin droit-bas) ---
    float gripSize = 16.0f; // Taille de la zone de drag pour le resize
    ImGui::SetNextWindowPos(ImVec2((float)winSize.x - gripSize, (float)winSize.y - gripSize));
    ImGui::SetNextWindowSize(ImVec2(gripSize, gripSize));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("##resize", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    ImGui::InvisibleButton("resize_drag", ImVec2(gripSize, gripSize));
    if (ImGui::IsItemActive())
    {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        int newWidth = std::max(100, (int)winSize.x + (int)delta.x);
        int newHeight = std::max(100, (int)winSize.y + (int)delta.y);
        window.setSize(sf::Vector2u(newWidth, newHeight));
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
}
