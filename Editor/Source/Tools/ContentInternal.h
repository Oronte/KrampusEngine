#pragma once
#include <imgui.h>
#include <filesystem>
#include <string>
#include <vector>

// What the files ContentPanel*.cpp share (the Content browser is split in several files to stay readable).
namespace Krampus::ContentInternal
{
	enum class Kind { Folder, Scene, Script, Texture, Audio, Font, Shape, Shader, Other };

	inline constexpr const char* DRAG_CONTENT = "KRAMPUS_CONTENT";

	// A move asked by a drop (tile or folder tree), done after the lists are drawn.
	inline std::filesystem::path g_moveSource;
	inline std::filesystem::path g_moveFolder;

	// ContentIcons.cpp: what a file is and how it is drawn
	bool IsIdentifier(const std::string& _name);
	void OpenWithSystem(const std::filesystem::path& _path);
	void ShowInExplorer(const std::filesystem::path& _path);
	std::string LowerExtension(const std::filesystem::path& _path);
	std::string ShaderNameOf(const std::filesystem::path& _file);
	std::string ShapeNameOf(const std::filesystem::path& _file);
	Kind KindOf(const std::filesystem::directory_entry& _entry);
	const char* NameOfKind(const Kind& _kind);
	void DrawIcon(ImDrawList* _draw, const ImVec2& _min, const ImVec2& _max, const std::filesystem::directory_entry& _entry, const Kind& _kind, const bool& _hovered);
	std::string FileSizeText(const std::filesystem::directory_entry& _entry);

	// ContentFiles.cpp: what can be done to the files (create, delete, rename, move, import)
	std::filesystem::path NewFolderIn(const std::filesystem::path& _parent);
	bool HasSubfolder(const std::filesystem::path& _path);
	bool CanDelete(const std::filesystem::path& _path);
	std::vector<std::filesystem::path> FilesToDelete(const std::filesystem::path& _path);
	bool MoveToRecycleBin(const std::vector<std::filesystem::path>& _files);
	bool IsScriptFile(const std::filesystem::path& _path);
	std::string RenameEntry(const std::filesystem::path& _path, const std::string& _newName, std::filesystem::path& _result);
	std::string MoveEntry(const std::filesystem::path& _source, const std::filesystem::path& _folder);
	std::string PrintablePath(const std::filesystem::path& _path);
	std::vector<std::filesystem::path> ImportDropped(const std::vector<std::filesystem::path>& _paths, const std::filesystem::path& _folder);
}
