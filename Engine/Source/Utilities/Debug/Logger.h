#pragma once
#include "Exception.h"
#include "Color.h"
#include "Utilities/System/Printable.h"
#include "Audio/Sound.h"

#define LOG(_verbosity, _msg)   Krampus::Logger::PrintLog(_verbosity, _msg, DEBUG_INFO)
#define LOG_MSG(_msg)           Krampus::Logger::PrintLog(Krampus::VerbosityType::Display, _msg, DEBUG_INFO)
#define LOG_WARNING(_msg)       Krampus::Logger::PrintLog(Krampus::VerbosityType::Warning, _msg, DEBUG_INFO)
#define LOG_ERROR(_msg)         Krampus::Logger::PrintLog(Krampus::VerbosityType::Error, _msg, DEBUG_INFO)
#define LOG_FATAL(_msg)         Krampus::Logger::PrintLog(Krampus::VerbosityType::Fatal, _msg, DEBUG_INFO)

namespace Krampus
{

    enum VerbosityType
    {
        VeryVerbose,    // Prints a verbose message to a log file (if veryVerboseLogging is enabled).
        Verbose,        // Prints a verbose message to a log file (if verboseLogging is enabled).
        Log,            // Prints a message to a log file (does not print to console).
        Display,        // Prints a message to console (and log file).
        Warning,        // Prints a warning to console (and log file).
        Error,          // Prints an error to console (and log file).
        Fatal,          // Always prints a fatal error to console (and log file) and throw an exception.
    };

    class VerbosityData
    {
        std::string     text;
        std::string     debug;
        Gradient        color;
        VerbosityType   type;

    public:
        VerbosityData(VerbosityType _type, const std::string& _text, const std::string& _debug)
            : type(_type), text(_text), debug(_debug) 
        {
            ComputeColor();
        }

        std::string RetrieveFullText(Bool _useColor = true) const;

    private:
        void ComputeColor();
    };

    class Logger
    {
        static inline std::string               logsDir             = "Content/Logs/";
        static inline std::string               logsFileName        = "log.txt";

        static inline std::string               logsPath            = logsDir + logsFileName;

        static inline std::queue<std::string>   logQueue;
        static inline std::queue<std::string>   consoleQueue;
        static inline std::mutex                queueMutex;
        static inline std::condition_variable   cv;
        static inline std::atomic<bool>         running             = false;
        static inline std::thread               logThread;

        static inline Sound                     msgNotif;
        static inline Sound                     warningNotif;
        static inline Sound                     errorNotif;
        static inline Sound                     fatalNotif;

    public:
        static inline Bool                      verboseLogging      = false;
        static inline Bool                      veryVerboseLogging  = false;
        static inline Bool                      canPlaySound        = true;
        static inline Bool                      canPlayMsgSound     = false;
        static inline Bool                      canPlayWarningSound = true;
        static inline Bool                      canPlayErrorSound   = true;
        static inline Bool                      canPlayFatalSound   = true;

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
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<std::decay_t<T>, bool> >>
        static void PrintLog(const VerbosityType& _type, const T& _number, const std::string& _debug = "")
        {
        #ifdef DEBUG
            PrintLog(_type, std::to_string(_number), _debug);
        #endif
        }
        static void PrintLog(const VerbosityType& _type, const std::string& _text, const std::string& _debug = "");
        static void PrintLog(const VerbosityType& _type, const IPrintable& _object, const std::string& _debug = "");
        static void PrintLog(const VerbosityType& _type, const IPrintable* _object, const std::string& _debug = "");

        static void Clear();

        static void PlayDebugSound(const VerbosityType& _type);
    };

}


//////////////////////////////////////////////////////
// 
//  LOG(VerbosityType::Warning, message);
//  LOG(VerbosityType::Fatal, FVector2(2.f, 3.f));
// 
//////////////////////////////////////////////////////