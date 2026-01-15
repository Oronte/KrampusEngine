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

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Ship"
        defines { "RELEASE", "SHIP" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter {}
