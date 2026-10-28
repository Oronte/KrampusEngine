#pragma once
#include <cstdint>
#include <filesystem>
#include <memory>
#include <istream>
#include <string>
#include <vector>

// The assets of a game (Content/) stored in ONE file, KrampusGame.pak, next to its exe (like the Paks of an Unreal game).
//
//   pack (count, index, data) | footer
//
// The packaging tool (Ship/Tools/PackTool.cpp) writes it, the game mounts it at start (Ship/Source/Main.cpp), and every loader of the
// engine asks AssetPack for the bytes of a file before looking on the disk. Without a mounted pack (the editor, a Debug or
// Release build) nothing changes: the files are read from the disk as always.
//
// This file uses nothing but the standard library, so that the packaging tool can use it too.

namespace Krampus
{

	class AssetPack
	{
	public:
		/// <summary>
		/// Looks for a pack at the end of a file (normally the exe itself) and loads it in memory.
		/// </summary>
		/// <returns>False if the file has no pack.</returns>
		static bool Mount(const std::filesystem::path& _exe);

		static bool IsMounted();

		/// <summary>
		/// The bytes of a packed file ("Content/Textures/Player.png", any slashes, any case). The pointer stays valid until the end of the
		/// program: SFML fonts and music that read from memory can keep it.
		/// </summary>
		/// <returns>False if the file is not in the pack (or there is no pack).</returns>
		static bool Find(const std::string& _path, const std::uint8_t*& _data, std::size_t& _size);

		/// <returns>True if the file is in the pack, or on the disk.</returns>
		static bool Exists(const std::filesystem::path& _path);

		/// <summary>
		/// The whole file, from the pack or else from the disk.
		/// </summary>
		static bool ReadAll(const std::filesystem::path& _path, std::vector<std::uint8_t>& _out);

		/// <summary>
		/// A stream on the file (binary), from the pack or else from the disk. nullptr if it does not exist.
		/// </summary>
		static std::unique_ptr<std::istream> OpenStream(const std::filesystem::path& _path);

		/// <summary>
		/// Writes a pak file: every file of _projectRoot/Content, except what a game does not need
		/// (Editor, Logs, Scripts, Screenshots, Saves, TODO.txt, *.pdb). Used by the packaging tool.
		/// </summary>
		/// <param name="_report">What was packed, or why it failed.</param>
		static bool BuildPak(const std::filesystem::path& _projectRoot, const std::filesystem::path& _outPak, std::string& _report);

		/// <summary>"Content\Scenes/Main.KA7" becomes "content/scenes/main.ka7".</summary>
		static std::string Normalize(const std::filesystem::path& _path);
	};
}
