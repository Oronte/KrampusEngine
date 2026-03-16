#pragma once
#include "Save/SaveNode.h"

struct SaveMetadata 
{
    String   name;
    String   filePath;
    LongLong timestamp    = LongLong(0LL);
    Short    version      = Short(0);
    Bool     isCompressed = Bool::False();
};

enum class SaveResult 
{
    Success,
    ErrorIO,
    ErrorCorrupted,
    ErrorVersionMismatch,
    ErrorChecksum,
    ErrorNotFound,
};

NO_DISCARD constexpr const char* ToString(SaveResult _result) noexcept 
{
    switch (_result)
    {
        case SaveResult::Success:              return "Success";
        case SaveResult::ErrorIO:              return "IO Error";
        case SaveResult::ErrorCorrupted:       return "File Corrupted";
        case SaveResult::ErrorVersionMismatch: return "Version Mismatch";
        case SaveResult::ErrorChecksum:        return "Checksum Failed";
        case SaveResult::ErrorNotFound:        return "File Not Found";
        default:                               return "Unknown";
    }
}

class SaveManager 
{
public:
    using EventCallback = std::function<void(const String& name, SaveResult result)>;

    static inline const Short  CURRENT_VERSION = Short(1);
#if SHIP
    static inline const String SAVE_DIRECTORY  = String("../../Content/Saves/");
#else
    static inline const String SAVE_DIRECTORY  = String(CONTENT_PATH + "Saves/");
#endif
    static inline const String SAVE_EXTENSION  = String(".ksave");

private:
    static String        saveDirectory;
    static EventCallback onSave;
    static EventCallback onLoad;

public:
    SaveManager()                              = delete;
    SaveManager(const SaveManager&)            = delete;
    SaveManager& operator=(const SaveManager&) = delete;

    NO_DISCARD static SaveResult Save(const String& name,
                                         const SaveNode& root,
                                         const Bool& compress = Bool::False());

    NO_DISCARD static SaveResult Load(const String& name,
                                         SaveNode& outRoot);

    NO_DISCARD static SaveNode LoadOrThrow(const String& name);

    NO_DISCARD static std::optional<SaveNode> TryLoad(const String& name);

    NO_DISCARD static Bool Exists(const String& name);
    NO_DISCARD static Bool Delete(const String& name);
    NO_DISCARD static Bool Rename(const String& oldName, const String& newName);
    NO_DISCARD static Bool Copy  (const String& src,     const String& dst);

    NO_DISCARD static std::vector<SaveMetadata>   ListSaves();
    NO_DISCARD static std::optional<SaveMetadata> GetMetadata(const String& name);

    static void EnsureSaveDirectoryExists();
    static void SetSaveDirectory(String dir);

    NO_DISCARD static std::filesystem::path GetSavePath(const String& name);

    static void SetOnSaveCallback(EventCallback cb);
    static void SetOnLoadCallback(EventCallback cb);
};
