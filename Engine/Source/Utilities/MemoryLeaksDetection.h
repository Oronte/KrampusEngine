#pragma once

#ifdef DEBUG

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace Krampus
{
    constexpr inline const char* LEAK_LOG_PATH = "Content/Logs/memory_leaks.txt";

    inline std::ofstream g_LeakFile;

    inline int __cdecl LeakReportHook(int /*reportType*/, char* message, int* returnValue)
    {
        if (returnValue) *returnValue = 0;

        if (g_LeakFile.is_open())
            g_LeakFile << message;

        return 1;
    }

    inline void SetupMemoryLeakDetection()
    {
        std::filesystem::create_directories(
            std::filesystem::path(LEAK_LOG_PATH).parent_path());

        g_LeakFile.open(LEAK_LOG_PATH, std::ios::out | std::ios::trunc);
        if (!g_LeakFile.is_open())
            return;

        const auto now = std::chrono::system_clock::now();
        const std::time_t t = std::chrono::system_clock::to_time_t(now);
        char timeBuf[64]{};
        ctime_s(timeBuf, sizeof(timeBuf), &t);

        g_LeakFile <<
            "========================================\n"
            " Krampus Engine - Memory Leak Report\n "
            + std::string(timeBuf) +
            "========================================\n\n";

        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, LeakReportHook);
    }

    inline void TeardownMemoryLeakDetection()
    {
        _CrtDumpMemoryLeaks();
        _CrtSetReportHook2(_CRT_RPTHOOK_REMOVE, LeakReportHook);

        if (g_LeakFile.is_open())
        {
            g_LeakFile << "\n[End of report]\n";
            g_LeakFile.close();
        }
    }

}

#endif