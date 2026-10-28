#pragma once
#include "EditorContext.h"
#include <imgui.h>

namespace Krampus
{

	/// <summary>
	/// Picture of an image file, loaded the first time it is asked for and kept (big files and unreadable ones give nullptr).
	/// Shared by the Content panel and the asset pickers of the Inspector.
	/// </summary>
	sf::Texture* ThumbnailOf(const std::filesystem::path& _path);

	/// <returns>The handle ImGui::Image needs for a texture.</returns>
	ImTextureID TextureId(const sf::Texture& _texture);

	/// <summary>
	/// Frees every picture. Must be called before the module is unloaded (OnDetach), not left to static destruction.
	/// </summary>
	void ClearThumbnails();
}
