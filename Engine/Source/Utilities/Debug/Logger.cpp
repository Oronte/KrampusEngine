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

    // The queues are only read with the lock held; it is released while a message is written.
    std::unique_lock _lock(queueMutex);
    while (true)
    {
        cv.wait(_lock, [] { return
            !logQueue.empty() ||
            !consoleQueue.empty() ||
            !running; });

        while (!logQueue.empty() || !consoleQueue.empty())
        {
            if (!consoleQueue.empty())
            {
                std::string _consoleText = std::move(consoleQueue.front());
                consoleQueue.pop();
                writing = true;
                _lock.unlock();
                std::cout << _consoleText << std::endl;
                _lock.lock();
                writing = false;
            }

            if (!logQueue.empty())
            {
                std::string _logText = std::move(logQueue.front());
                logQueue.pop();
                writing = true;
                _lock.unlock();
                _file << _logText << std::endl;
                _file.flush();
                _lock.lock();
                writing = false;
            }
        }
        drainedCv.notify_all();

        if (!running) break;    // everything queued before the shutdown is written
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
    if (logThread.joinable()) return;   // already running: a second std::thread on top of a joinable one would terminate the program

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
    {
        // Under the lock: otherwise the thread can test 'running' just before it changes and then wait forever.
        std::lock_guard _lock(queueMutex);
        running = false;
    }
    cv.notify_all();
    if (logThread.joinable())
        logThread.join();

    // Release audio now: destroying the last sound at DLL unload would stop the audio thread under the loader lock and deadlock.
    msgNotif = Sound();
    warningNotif = Sound();
    errorNotif = Sound();
    fatalNotif = Sound();
#endif
}

void Krampus::Logger::Flush()
{
#ifdef DEBUG
    if (!logThread.joinable() || std::this_thread::get_id() == logThread.get_id()) return;

    std::unique_lock _lock(queueMutex);
    drainedCv.wait_for(_lock, std::chrono::seconds(2), []
    {
        return !running || (logQueue.empty() && consoleQueue.empty() && !writing);
    });
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
        const std::string _plainText = _verbosity.RetrieveFullText(false);
        EnqueueLog(_plainText);

        if (collectEntries)
        {
            std::lock_guard _lock(entriesMutex);
            pendingEntries.push_back({ _type, _plainText });
        }

        if (CanPrintInConsole(_type)) EnqueueConsole(_verbosity.RetrieveFullText(canColorText));

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

void Krampus::Logger::DrainEntries(std::vector<Entry>& _out)
{
    std::lock_guard _lock(entriesMutex);
    _out.insert(_out.end(), std::make_move_iterator(pendingEntries.begin()), std::make_move_iterator(pendingEntries.end()));
    pendingEntries.clear();
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
