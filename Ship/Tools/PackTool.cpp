// PackTool <project root> <output.pak>
// Writes <output.pak>: the files of <project root>/Content in one file that the game mounts at start (see Engine/Source/Core/AssetPack.h).
// Run by the Package target (Ship/CMakeLists.txt).

#include "Core/AssetPack.h"
#include <iostream>

int main(int _argc, char** _argv)
{
	if (_argc < 3)
	{
		std::cerr << "usage: PackTool <project root> <output.pak>" << std::endl;
		return 2;
	}

	std::string _report;
	if (!Krampus::AssetPack::BuildPak(_argv[1], _argv[2], _report))
	{
		std::cerr << "PackTool error: " << _report << std::endl;
		return 1;
	}

	std::cout << "Package: " << _report << std::endl;
	return 0;
}
