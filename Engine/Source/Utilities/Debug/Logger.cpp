#include "Logger.h"
#include "Managers/TimerManager.h"
#include "Managers/AudioManager.h"

using namespace Krampus;

std::string VerbosityData::RetrieveFullText(Bool _useColor) const
{
    static constexpr const char* _table[] =
    {
        "VeryVerbose", "Verbose", "Log", "Display",
        "Warning", "Error", "Fatal"
    };

    std::string _outString =
        "[" + TimerManager::GetCurrentRealTime() + "]" +
        " " + _table[type] + ": " + text + " " + debug;

    return _useColor ? color.GradientString(_outString) : _outString;
}

void Krampus::VerbosityData::ComputeColor()
{
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

    color = _verbosityColors[type];
}






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
#ifdef DEBUG
    Reset();
    running = true;
    logThread = std::thread(LoggingThread);

    msgNotif = M_AUDIO.CreateSound("_internal/MsgLogNotif", AudioExtensionType::MP3);
    warningNotif = M_AUDIO.CreateSound("_internal/WarningLogNotif", AudioExtensionType::MP3);
    errorNotif = M_AUDIO.CreateSound("_internal/ErrorLogNotif", AudioExtensionType::MP3);
    fatalNotif = M_AUDIO.CreateSound("_internal/FatalLogNotif", AudioExtensionType::WAV);

    msgNotif.SetSpatializationEnabled(false);
    warningNotif.SetSpatializationEnabled(false);
    errorNotif.SetSpatializationEnabled(false);
    fatalNotif.SetSpatializationEnabled(false);
#endif
}

void Krampus::Logger::Shutdown()
{
#ifdef DEBUG
    running = false;
    cv.notify_all();
    if (logThread.joinable())
        logThread.join();
#endif
}

void Logger::Reset()
{
#ifdef DEBUG
    std::ofstream(logsPath, std::ios_base::trunc);
#endif
}

void Logger::PrintLog(const VerbosityType& _type, const std::string& _text, const std::string& _debug)
{
#ifdef DEBUG
    if (!running) 
        std::cout << "You printed a log but the Logger is not running (call => engine::Logger::Init()) " << DEBUG_INFO << std::endl;

    if (CanPrintInLog(_type))
    {
        const VerbosityData& _verbosity = VerbosityData(_type, _text, _debug);
        EnqueueLog(_verbosity.RetrieveFullText(false));

        if (CanPrintInConsole(_type)) EnqueueConsole(_verbosity.RetrieveFullText(true));

        PlayDebugSound(_type);
    }

    if (_type == VerbosityType::Fatal) THROW_EXCEPTION("Fatal exception occurred");
#endif
}

void Krampus::Logger::PrintLog(const VerbosityType& _type, const IPrintable& _object, const std::string& _debug)
{
#ifdef DEBUG
    PrintLog(_type, _object.ToString(), _debug);
#endif
}

void Krampus::Logger::PrintLog(const VerbosityType& _type, const IPrintable* _object, const std::string& _debug)
{
#ifdef DEBUG
    if (!_object)
    {
        LOG(Krampus::VerbosityType::Error, "You try to print a IPrintable ptr but it's nullptr");
        return;
    }
    PrintLog(_type, _object->ToString(), _debug);
#endif
}

void Krampus::Logger::Clear()
{
#ifdef DEBUG
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
#endif
}

void Krampus::Logger::PlayDebugSound(const VerbosityType& _type)
{
    if (!canPlaySound)
        return;

    if (_type == VerbosityType::Display)
    {
        if (msgNotif.IsAvaliable())
        {
            if (canPlayMsgSound) msgNotif.Play();
        }
    }
    else if (_type == VerbosityType::Warning)
    {
        if (warningNotif.IsAvaliable())
        {
            if (canPlayWarningSound) warningNotif.Play();
        }
    }
    else if (_type == VerbosityType::Error)
    {
        if (errorNotif.IsAvaliable())
        {
            if (canPlayErrorSound) errorNotif.Play();
        }
    }
    else if (_type == VerbosityType::Fatal)
    {
        if (fatalNotif.IsAvaliable())
        {
            if (canPlayFatalSound) fatalNotif.Play();
        }
    }
}
