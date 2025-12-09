#include "Logger.h"
#include "Managers/TimerManager.h"

using namespace Krampus;

Krampus::VerbosityData::VerbosityData(const VerbosityType& _type, const std::string& _text, const std::string& _debug, const bool& _useDebug)
{
    ComputeUseDebug(_type);
    ComputeColor(_type);
    ComputePrefix(_type);
    text = _text;
    debug = _debug;
}

/// VerbosityData
std::string VerbosityData::RetrieveFullText(const bool& _useColor, const bool& _useTime) const
{
    std::string _fullText;

    if (_useTime)
    {
        _fullText += "[" + M_TIMER.GetCurrentRealTime() + "]";
    }

    _fullText += " " + prefix + ": " + text;

    if (USE_DEBUG || useDebug)
    {
        _fullText += " " + debug;
    }

    return _useColor ? color.GradientString(_fullText) : _fullText;
}

void Krampus::VerbosityData::ComputeUseDebug(const VerbosityType& _type)
{
    useDebug = std::set<VerbosityType>({ VerbosityType::Error, VerbosityType::Error }).contains(_type);
}

void Krampus::VerbosityData::ComputeColor(const VerbosityType& _type)
{
    if (_type >= VerbosityType::COUNT)
        THROW_EXCEPTION(std::format("Invalid VerbosityType, _type = {} and must be < {}", CAST(int, _type), CAST(int, VerbosityType::COUNT)));

    const std::vector<Gradient>& _verbosityColors =
    {
        Gradient(Color(27, 27, 33), Color(37, 37, 51)),         //VERY VERBOSE
        Gradient(Color(55, 55, 61), Color(69, 69, 93)),         //VERBOSE
        Gradient(Color(100, 100, 119), Color(143, 143, 194)),   //LOG
        Gradient(Color(221, 221, 246), Color(122, 122, 236)),   //DISPLAY
        Gradient(Color(255, 231, 0), Color(255, 76, 17)),       //WARNING
        Gradient(Color(193, 6, 11), Color(249, 56, 67)),        //ERROR
        Gradient(Color(255, 0, 95), Color(118, 37, 184)),       //FATAL
    };

    color = _verbosityColors[CAST(int, _type)];
}

void Krampus::VerbosityData::ComputePrefix(const VerbosityType& _type)
{
    if (_type >= VerbosityType::COUNT)
        THROW_EXCEPTION(std::format("Invalid VerbosityType ! _type = {} and must be < {}", CAST(int, _type), CAST(int, VerbosityType::COUNT)));

    const std::vector<std::string>& _verbosityTexts =
    {
        "VeryVerbose",
        "Verbose",
        "Log",
        "Display",
        "Warning",
        "Error",
        "Fatal",
    };

    prefix = _verbosityTexts[CAST(int, _type)];
}


/// Logger
void Krampus::Logger::LoggingThread()
{
    std::filesystem::create_directories(logsDir);
    std::ofstream _file(logsPath, std::ios_base::app);

    while (running || !logQueue.empty())
    {
        std::unique_lock _lock(queueMutex);
        cv.wait(_lock, [] { return 
            !logQueue.empty() ||
            !consoleQueue.empty() ||
            !running; });

        while (!logQueue.empty() || !consoleQueue.empty())
        {
            if (!consoleQueue.empty())
            {
                std::string _consoleText = consoleQueue.front();
                consoleQueue.pop();
                _lock.unlock();
                std::cout << _consoleText << std::endl;
                _lock.lock();
            }

            if (!logQueue.empty())
            {
                std::string _logText = logQueue.front();
                logQueue.pop();
                _lock.unlock();
                _file << _logText << std::endl;
                _file.flush();
                _lock.lock();
            }
        }
    }
}

void Krampus::Logger::EnqueueLog(const std::string& _logText)
{
    std::lock_guard _lock(queueMutex);
    logQueue.push({ _logText });
    cv.notify_one();
}

void Krampus::Logger::EnqueueConsole(const std::string& _consoleText)
{
    std::lock_guard _lock(queueMutex);
    consoleQueue.push({ _consoleText });
    cv.notify_one();
}

bool Krampus::Logger::CanPrintInLog(const VerbosityType& _type)
{
    if (_type >= VerbosityType::Log) return true;

    if (verboseLogging)
    {
        if (_type >= VerbosityType::Verbose) return true;
        if (veryVerboseLogging)
        {
            if (_type >= VerbosityType::VeryVerbose) return true;
        }
    }
    return false;
}

bool Krampus::Logger::CanPrintInConsole(const VerbosityType& _type)
{
    return _type > VerbosityType::Log;
}

void Logger::Init()
{
    Reset();
    running = true;
    logThread = std::thread(LoggingThread);
}

void Krampus::Logger::Shutdown()
{
    running = false;
    cv.notify_all();
    if (logThread.joinable())
        logThread.join();
}

void Logger::Reset()
{
    std::ofstream(logsPath, std::ios_base::trunc);
}

void Logger::PrintLog(const VerbosityType& _type, const std::string& _text, const std::string& _debug)
{
    if (!running) 
        std::cout << "You printed a log but the Logger is not running (call => engine::Logger::Init()) " << DEBUG_INFO << std::endl;

    if (CanPrintInLog(_type))
    {
        const VerbosityData& _verbosity = VerbosityData(_type, _text, _debug);
        EnqueueLog(_verbosity.RetrieveFullText(false));

        if (CanPrintInConsole(_type)) EnqueueConsole(_verbosity.RetrieveFullText(true));
    }

    if (_type == VerbosityType::Fatal) THROW_EXCEPTION("Fatal exception occurred");
}

void Krampus::Logger::PrintLog(const VerbosityType& _type, const IPrintable& _object, const std::string& _debug)
{
    PrintLog(_type, _object.ToString(), _debug);
}

void Krampus::Logger::PrintLog(const VerbosityType& _type, const IPrintable* _object, const std::string& _debug)
{
    if (!_object)
    {
        LOG(Krampus::VerbosityType::Error, "You try to print a IPrintable ptr but it's nullptr");
        return;
    }
    PrintLog(_type, _object->ToString(), _debug);
}
