project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    files { "Source/**.h", "Source/**.cpp" }

    includedirs {
        "Source",
        RootDir .. "/ThirdParty/SFML-3.0.0/include",
        RootDir .. "/ThirdParty/rapidjson",
        RootDir .. "/ThirdParty/imgui"
    }

    libdirs {
        RootDir .. "/ThirdParty/SFML-3.0.0/lib"
    }

    targetdir (RootDir .. "/Bin/" .. OutputDir .. "/%{prj.name}")
    objdir    (RootDir .. "/Bin/Intermediates/" .. OutputDir .. "/%{prj.name}")

    defines { 'PROJECT_ROOT=\"' .. RootDir:gsub("\\","/") .. '\"' }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines { "DEBUG" }
        links {
            "sfml-graphics-d", "sfml-window-d",
            "sfml-system-d", "sfml-audio-d",
            "sfml-network-d"
        }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        links {
            "sfml-graphics", "sfml-window",
            "sfml-system", "sfml-audio",
            "sfml-network"
        }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter {}
