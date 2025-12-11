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
        RootDir .. "/ThirdParty/SFML-3.0.0/lib",
        RootDir .. "/ThirdParty/imgui/ImGui/binaries"
    }

    targetdir (RootDir .. "/Bin/" .. OutputDir .. "/%{prj.name}")
    objdir    (RootDir .. "/Bin/Intermediates/" .. OutputDir .. "/%{prj.name}")

    defines { 'PROJECT_ROOT_DIR=\"' .. RootDir:gsub("\\","/") .. '\"' }

    filter "system:windows"
        systemversion "latest"
        links { "opengl32" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        links {
            "sfml-graphics-d", "sfml-window-d",
            "sfml-system-d", "sfml-audio-d",
            "sfml-network-d",
            "ImGuiD"
        }
        runtime "Debug"
        symbols "On"
        postbuildcommands {
            'copy /Y "%{RootDir}\\ThirdParty\\imgui\\ImGui\\binaries\\ImGuiD.lib" "%{cfg.targetdir}"'
        }

    filter "configurations:Release"
        defines { "RELEASE" }
        links {
            "sfml-graphics", "sfml-window",
            "sfml-system", "sfml-audio",
            "sfml-network",
            "ImGuiR"
        }
        runtime "Release"
        optimize "On"
        symbols "On"
        postbuildcommands {
            'copy /Y "%{RootDir}\\ThirdParty\\imgui\\ImGui\\binaries\\ImGuiR.lib" "%{cfg.targetdir}"'
        }

    filter {}
