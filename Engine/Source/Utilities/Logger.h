#pragma once
#include "Exception.h"
#include "Color.h"
#include "System/Printable.h"

#define CAN_DEBUG
#ifdef CAN_DEBUG
#define USE_DEBUG 1
#else
#define USE_DEBUG 0
#endif // DEBUG

#define LOG(_verbosity, _msg)   Krampus::Logger::PrintLog(_verbosity, _msg, DEBUG_INFO)
#define LOG_MSG(_msg)           Krampus::Logger::PrintLog(Krampus::VerbosityType::Display, _msg, DEBUG_INFO)
#define LOG_WARNING(_msg)       Krampus::Logger::PrintLog(Krampus::VerbosityType::Warning, _msg, DEBUG_INFO)
#define LOG_ERROR(_msg)         Krampus::Logger::PrintLog(Krampus::VerbosityType::Error, _msg, DEBUG_INFO)
#define LOG_FATAL(_msg)         Krampus::Logger::PrintLog(Krampus::VerbosityType::Fatal, _msg, DEBUG_INFO)

namespace Krampus
{

    enum class VerbosityType
    {
        VeryVerbose,    // Prints a verbose message to a log file (if veryVerboseLogging is enabled).
        Verbose,        // Prints a verbose message to a log file (if verboseLogging is enabled).
        Log,            // Prints a message to a log file (does not print to console).
        Display,        // Prints a message to console (and log file).
        Warning,        // Prints a warning to console (and log file).
        Error,          // Prints an error to console (and log file).
        Fatal,          // Always prints a fatal error to console (and log file) and throw an exception.

        COUNT
    };

    class VerbosityData
    {
        Gradient color;
        std::string prefix;
        std::string text;
        std::string debug;
        bool useDebug;

    public:
        VerbosityData(const VerbosityType& _type, const std::string& _text, const std::string& _debug,
            const bool& _useDebug = false);

        std::string RetrieveFullText(const bool& _useColor = true, const bool& _useTime = true) const;

    private:
        void ComputeUseDebug(const VerbosityType& _type);
        void ComputeColor(const VerbosityType& _type);
        void ComputePrefix(const VerbosityType& _type);
    };

    class Logger
    {
        static INLINE std::string logsDir = CONTENT_PATH + "Logs/";
        static INLINE std::string logsFileName = "log.txt";

        static INLINE std::string logsPath = logsDir + logsFileName;

        static INLINE std::queue<std::string> logQueue;
        static INLINE std::queue<std::string> consoleQueue;
        static INLINE std::mutex queueMutex;
        static INLINE std::condition_variable cv;
        static INLINE std::atomic<bool> running = false;
        static INLINE std::thread logThread;

    public:
        static INLINE bool verboseLogging = false;
        static INLINE bool veryVerboseLogging = false;

    private:
        static void LoggingThread();
        static void EnqueueLog(const std::string& _logText);
        static void EnqueueConsole(const std::string& _consoleText);
        static bool CanPrintInLog(const VerbosityType& _type);
        static bool CanPrintInConsole(const VerbosityType& _type);

    public:
        static void Init();
        static void Shutdown();
        static void Reset();
        static void PrintLog(const VerbosityType& _type, const std::string& _text, const std::string& _debug = "");
        static void PrintLog(const VerbosityType& _type, const IPrintable& _object, const std::string& _debug = "");
        static void PrintLog(const VerbosityType& _type, const IPrintable* _object, const std::string& _debug = "");
    };

}


//////////////////////////////////////////////////////
// 
//  LOG(VerbosityType::Warning, message);
//  LOG(VerbosityType::Fatal, FVector2(2.f, 3.f));
// 
//////////////////////////////////////////////////////