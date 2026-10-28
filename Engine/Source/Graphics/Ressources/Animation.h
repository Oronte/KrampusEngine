#pragma once
#include "Graphics/Shapes/ShapeObject.h"
#include "Utilities/System/Timer.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

    /// <summary>
    /// Traversal order when automatically generating animation frames from a spritesheet.
    /// </summary>
    enum class ReadDirection
    {
        RD_ROW,
        RD_ROW_REVERSE,
        RD_COLUMN,
        RD_COLUMN_REVERSE,
    };

    /// <summary>
    /// Configuration data defining animation frames, loop settings, and playback duration.
    /// </summary>
    struct AnimationData
    {
        Bool                canLoop     = false;
        Int                 count       = 0;
        Float               duration    = 0.0f;
        std::vector<IRect>  sprites;

        /// <summary>
        /// Default constructor.
        /// </summary>
        AnimationData() = default;

        /// <summary>
        /// Constructs animation data automatically sliced in rows or columns from an initial frame rect.
        /// </summary>
        /// <param name="_count">Total number of frames.</param>
        /// <param name="_duration">Total animation duration in seconds.</param>
        /// <param name="_spriteData">Bounding rectangle of the first sprite frame.</param>
        /// <param name="_canLoop">Whether playback should loop.</param>
        /// <param name="_direction">Direction to advance across spritesheet.</param>
        AnimationData(const Int& _count, const Float& _duration, const IRect& _spriteData,
            const Bool& _canLoop = true, const ReadDirection& _direction = ReadDirection::RD_ROW);

        /// <summary>
        /// Constructs animation data from an explicit list of frame bounding rectangles.
        /// </summary>
        /// <param name="_duration">Total animation duration in seconds.</param>
        /// <param name="_spritesData">Vector of sub-rectangles for each frame.</param>
        /// <param name="_canLoop">Whether playback should loop.</param>
        AnimationData(const Float& _duration, const std::vector<IRect>& _spritesData,
            const Bool& _canLoop = true);
    };

    /// <summary>
    /// One animation described by plain values, so it can be edited in the Inspector and saved in a scene:
    /// frames are cut from a sprite sheet, starting at frameOrigin, each frameSize big.
    /// </summary>
    struct AnimationClip
    {
        String      name        = "Default";
        Int         frameCount  = 0;            // 0 = no animation
        Float       duration    = 1.0f;         // seconds for the whole animation
        FVector2    frameOrigin = FVector2(0.0f, 0.0f);
        FVector2    frameSize   = FVector2(64.0f, 64.0f);
        Int         direction   = 0;            // ReadDirection: 0 row, 1 row reversed, 2 column, 3 column reversed
        Bool        loop        = true;

        AnimationData ToData() const
        {
            return AnimationData(frameCount, duration,
                IRect(IVector2(CAST(int, frameOrigin.x), CAST(int, frameOrigin.y)), IVector2(CAST(int, frameSize.x), CAST(int, frameSize.y))),
                loop, CAST(ReadDirection, CAST(int, direction)));
        }
    };

    /// <summary>
    /// Controls timed frame sequencing on a 2D ShapeObject sprite.
    /// </summary>
    class Animation : public KrampusObject
    {
    public:
        /// <summary>
        /// Playback state of the animation.
        /// </summary>
        enum class State
        {
            Playing,
            Stoped
        };

    protected:
        Int             currentIndex = 0;
        String          name;
        AnimationData   data;
        ShapeObject*    shape;
        Timer*          timer;
        State           state;

    public:
        /// <summary>
        /// Gets the current playback state of the animation.
        /// </summary>
        /// <returns>Current State enum value.</returns>
        State GetState()
        {
            return state;
        }

        /// <summary>
        /// Frame notification events mapped by frame index, triggered when that frame is displayed.
        /// </summary>
        std::unordered_map<Int, Event<>> notifies;

    protected:
        /// <summary>
        /// Checks whether currentIndex is within bounds of available frames.
        /// </summary>
        /// <returns>True if index is valid.</returns>
        inline Bool IsValidIndex() const
        {
            return currentIndex < data.count;
        }

        /// <summary>
        /// Calculates per-frame duration in seconds using current frame data.
        /// </summary>
        /// <returns>Duration per frame.</returns>
        inline Float ComputeDuration()
        {
            IRect* _spriteData = GetSpriteData();
            if (!_spriteData)
            {
                LOG_ERROR("There is no sprite data to Compute the duration");
                return 0.0f;
            }
            return ComputeDuration(*_spriteData);
        }

        /// <summary>
        /// Calculates per-frame duration in seconds for a specific frame rect.
        /// </summary>
        /// <param name="_spriteData">Frame rectangle.</param>
        /// <returns>Duration per frame in seconds.</returns>
        inline Float ComputeDuration(const IRect& _spriteData) const
        {
            const int& _count = data.count;
            if (_count == 0)
            {
                LOG_ERROR("Can't Compute the duration if sprite count == 0");
                return 0.0f;
            }
            return data.duration / CAST(float, _count);
        }

        /// <summary>
        /// Gets the bounding rectangle of the current frame.
        /// </summary>
        /// <returns>Pointer to current IRect, or nullptr if empty.</returns>
        inline IRect* GetSpriteData()
        {
            if (data.sprites.empty()) return nullptr;

            const int _index = currentIndex == 0 ? 0 : (int)currentIndex - 1;
            return &data.sprites[_index];
        }

    public:
        /// <summary>
        /// Gets the name of the animation.
        /// </summary>
        /// <returns>Animation name string.</returns>
        inline String GetName() const
        {
            return name;
        }

    public:
        /// <summary>
        /// Constructs an Animation bound to a ShapeObject.
        /// </summary>
        /// <param name="_engine">Pointer to the Engine.</param>
        /// <param name="_name">Animation name identifier.</param>
        /// <param name="_shape">Target ShapeObject.</param>
        /// <param name="_data">Animation frame configuration.</param>
        Animation(Engine* _engine, const String& _name, ShapeObject* _shape, const AnimationData& _data);

        /// <summary>
        /// Destructs the Animation and releases its timer.
        /// </summary>
        ~Animation();

    protected:
        /// <summary>
        /// Advances to the next frame and triggers notifications.
        /// </summary>
        void Update();

        /// <summary>
        /// Reschedules the frame timer for the next frame.
        /// </summary>
        /// <param name="_spriteData">Current frame data.</param>
        void UpdateTimer(const IRect& _spriteData);

        /// <summary>
        /// Resets frame counter to the start of the sequence.
        /// </summary>
        void Reset();

    public:
        /// <summary>
        /// Starts animation playback from the first frame.
        /// </summary>
        void Start();

        /// <summary>
        /// Resumes animation playback from a paused state.
        /// </summary>
        void Resume();

        /// <summary>
        /// Pauses animation playback at the current frame.
        /// </summary>
        void Pause();

        /// <summary>
        /// Stops animation playback and resets to the first frame.
        /// </summary>
        void Stop();
    };

}
