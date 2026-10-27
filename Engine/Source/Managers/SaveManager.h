#pragma once
#include "Save/SaveNode.h"

/// <summary>
/// Metadata descriptor describing a saved game file.
/// </summary>
struct SaveMetadata 
{
    /// <summary>Name identifier of the save file.</summary>
    String   name;

    /// <summary>Full filesystem path of the save file.</summary>
    String   filePath;

    /// <summary>Unix timestamp when the save was created.</summary>
    LongLong timestamp    = LongLong(0LL);

    /// <summary>Schema version number of the save file.</summary>
    Short    version      = Short(0);

    /// <summary>Whether the save data is compressed.</summary>
    Bool     isCompressed = Bool::False();
};

/// <summary>
/// Status codes returned by save and load operations.
/// </summary>
enum class SaveResult 
{
    Success,
    ErrorIO,
    ErrorCorrupted,
    ErrorVersionMismatch,
    ErrorChecksum,
    ErrorNotFound,
};

/// <summary>
/// Converts a SaveResult status code to a human-readable English string.
/// </summary>
/// <param name="_result">The SaveResult code.</param>
/// <returns>Status description string.</returns>
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

/// <summary>
/// Static manager responsible for serializing, compressing, checksumming, and loading save game trees.
/// </summary>
class SaveManager 
{
public:
    /// <summary>Callback type signature for save/load completion notifications.</summary>
    using EventCallback = std::function<void(const String& name, SaveResult result)>;

    /// <summary>Current save format schema version.</summary>
    static inline const Short  CURRENT_VERSION = Short(1);

#if SHIP
    /// <summary>Default save directory in ship build.</summary>
    static inline const String SAVE_DIRECTORY  = String("../../Content/Saves/");
#else
    /// <summary>Default save directory in dev build.</summary>
    static inline const String SAVE_DIRECTORY  = String(CONTENT_PATH + "Saves/");
#endif

    /// <summary>File extension for Krampus save files.</summary>
    static inline const String SAVE_EXTENSION  = String(".ksave");

private:
    static String        saveDirectory;
    static EventCallback onSave;
    static EventCallback onLoad;

public:
    SaveManager()                              = delete;
    SaveManager(const SaveManager&)            = delete;
    SaveManager& operator=(const SaveManager&) = delete;

    /// <summary>
    /// Serializes a SaveNode tree to a file on disk with optional compression.
    /// </summary>
    /// <param name="name">Save slot name.</param>
    /// <param name="root">Root SaveNode tree to serialize.</param>
    /// <param name="compress">Whether to compress data.</param>
    /// <returns>SaveResult status code.</returns>
    NO_DISCARD static SaveResult Save(const String& name,
                                         const SaveNode& root,
                                         const Bool& compress = Bool::False());

    /// <summary>
    /// Deserializes a SaveNode tree from a save file.
    /// </summary>
    /// <param name="name">Save slot name.</param>
    /// <param name="outRoot">Target SaveNode to populate.</param>
    /// <returns>SaveResult status code.</returns>
    NO_DISCARD static SaveResult Load(const String& name,
                                         SaveNode& outRoot);

    /// <summary>
    /// Loads a save tree or throws an exception on failure.
    /// </summary>
    /// <param name="name">Save slot name.</param>
    /// <returns>Loaded SaveNode root.</returns>
    NO_DISCARD static SaveNode LoadOrThrow(const String& name);

    /// <summary>
    /// Attempts to load a save file, returning std::nullopt on failure.
    /// </summary>
    /// <param name="name">Save slot name.</param>
    /// <returns>Optional SaveNode.</returns>
    NO_DISCARD static std::optional<SaveNode> TryLoad(const String& name);

    /// <summary>
    /// Checks whether a save file exists on disk.
    /// </summary>
    /// <param name="name">Save slot name.</param>
    /// <returns>True if file exists.</returns>
    NO_DISCARD static Bool Exists(const String& name);

    /// <summary>
    /// Deletes a save file from disk.
    /// </summary>
    /// <param name="name">Save slot name.</param>
    /// <returns>True if successfully deleted.</returns>
    NO_DISCARD static Bool Delete(const String& name);

    /// <summary>
    /// Renames an existing save file.
    /// </summary>
    /// <param name="oldName">Original save slot name.</param>
    /// <param name="newName">New save slot name.</param>
    /// <returns>True if renamed successfully.</returns>
    NO_DISCARD static Bool Rename(const String& oldName, const String& newName);

    /// <summary>
    /// Copies a save file to a new slot name.
    /// </summary>
    /// <param name="src">Source save name.</param>
    /// <param name="dst">Destination save name.</param>
    /// <returns>True if copied successfully.</returns>
    NO_DISCARD static Bool Copy  (const String& src,     const String& dst);

    /// <summary>
    /// Scans the saves directory and returns metadata for all valid save files.
    /// </summary>
    /// <returns>Vector of SaveMetadata structures.</returns>
    NO_DISCARD static std::vector<SaveMetadata>   ListSaves();

    /// <summary>
    /// Reads metadata from a save file without deserializing the full payload.
    /// </summary>
    /// <param name="name">Save slot name.</param>
    /// <returns>Optional SaveMetadata.</returns>
    NO_DISCARD static std::optional<SaveMetadata> GetMetadata(const String& name);

    /// <summary>
    /// Ensures that the save files directory exists on disk, creating it if needed.
    /// </summary>
    static void EnsureSaveDirectoryExists();

    /// <summary>
    /// Overrides the directory path used to store save files.
    /// </summary>
    /// <param name="dir">Directory path.</param>
    static void SetSaveDirectory(String dir);

    /// <summary>
    /// Resolves the full filesystem path for a named save file.
    /// </summary>
    /// <param name="name">Save slot name.</param>
    /// <returns>std::filesystem::path to file.</returns>
    NO_DISCARD static std::filesystem::path GetSavePath(const String& name);

    /// <summary>
    /// Sets an event callback invoked upon save completion.
    /// </summary>
    /// <param name="cb">Callback function.</param>
    static void SetOnSaveCallback(EventCallback cb);

    /// <summary>
    /// Sets an event callback invoked upon load completion.
    /// </summary>
    /// <param name="cb">Callback function.</param>
    static void SetOnLoadCallback(EventCallback cb);
};
