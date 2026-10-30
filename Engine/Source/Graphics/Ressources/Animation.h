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
            const Bool& _canLoop = true, const ReadDirection& _direction = ReadDirection::RD_ROW, const Int& _wrap = 0);

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
        Int         wrap        = 0;            // frames per line before going to the next one (down for a row, right for a column); 0 = one single line
        Bool        loop        = true;

        AnimationData ToData() const
        {
            return AnimationData(frameCount, duration,
                IRect(IVector2(CAST(int, frameOrigin.x), CAST(int, frameOrigin.y)), IVector2(CAST(int, frameSize.x), CAST(int, frameSize.y))),
                loop, CAST(ReadDirection, CAST(int, direction)), wrap);
        }
    };

    /// <summary>
    /// The list of clips of an animation component, written in one string so it can be saved in a scene:
    /// clips are separated by ';', the values of a clip by '|' (name|frameCount|duration|originX|originY|width|height|direction|loop|wrap).
    /// </summary>
    struct AnimationClips
    {
        static std::string Encode(const std::vector<AnimationClip>& _clips)
        {
            std::string _text;
            for (const AnimationClip& _clip : _clips)
            {
                std::string _name = _clip.name.StdString();
                std::replace_if(_name.begin(), _name.end(), [](const char _c) { return _c == '|' || _c == ';'; }, '_');
                if (!_text.empty()) _text += ';';
                _text += std::format("{}|{}|{}|{}|{}|{}|{}|{}|{}|{}", _name, static_cast<int>(_clip.frameCount), static_cast<float>(_clip.duration),
                    _clip.frameOrigin.x, _clip.frameOrigin.y, _clip.frameSize.x, _clip.frameSize.y, static_cast<int>(_clip.direction), static_cast<bool>(_clip.loop) ? 1 : 0, static_cast<int>(_clip.wrap));
            }
            return _text;
        }

        static std::vector<AnimationClip> Decode(const std::string& _text)
        {
            std::vector<AnimationClip> _clips;
            size_t _start = 0;
            while (_start <= _text.size() && !_text.empty())
            {
                size_t _end = _text.find(';', _start);
                if (_end == std::string::npos) _end = _text.size();

                std::vector<std::string> _values;
                size_t _from = _start;
                for (size_t _bar = _text.find('|', _from); _bar != std::string::npos && _bar < _end; _bar = _text.find('|', _from))
                {
                    _values.push_back(_text.substr(_from, _bar - _from));
                    _from = _bar + 1;
                }
                _values.push_back(_text.substr(_from, _end - _from));

                if (_values.size() >= 9)
                {
                    try
                    {
                        AnimationClip _clip;
                        _clip.name = String(_values[0]);
                        _clip.frameCount = Int(std::max(0, std::stoi(_values[1])));
                        _clip.duration = Float(std::max(0.01f, std::stof(_values[2])));
                        _clip.frameOrigin = FVector2(std::stof(_values[3]), std::stof(_values[4]));
                        _clip.frameSize = FVector2(std::max(1.f, std::stof(_values[5])), std::max(1.f, std::stof(_values[6])));
                        _clip.direction = Int(std::clamp(std::stoi(_values[7]), 0, 3));
                        _clip.loop = Bool(_values[8] != "0");
                        if (_values.size() > 9) _clip.wrap = Int(std::max(0, std::stoi(_values[9])));
                        _clips.push_back(_clip);
                    }
                    catch (const std::exception&) { }	// a damaged clip is skipped
                }

                if (_end >= _text.size()) break;
                _start = _end + 1;
            }
            return _clips;
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
        Timer*          timer = nullptr;
        State           state;
        Int             completedCycles = 0;     // times the last frame was passed since Start (loops count each time)

    public:
        /// <summary>
        /// Gets the current playback state of the animation.
        /// </summary>
        /// <returns>Current State enum value.</returns>
        State GetState()
        {
            return state;
        }

        /// <summary>How many times the animation reached its end since it was started (0: still in its first pass).</summary>
        Int GetCompletedCycles() const
        {
            return completedCycles;
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

        /// <summary>
        /// Advances to the next frame and triggers notifications.
        /// </summary>
        void Update();

    protected:
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


    /// <summary>
    /// Makes the animations described by clips on a component that keeps them in a map: removes what the previous clips made
    /// (appliedClips), adds the new ones through _add, and makes startAnimation (else the first clip) the current animation.
    /// Shared by AnimationComponent, AnimationComponentSM and AnimationWidgetComponent.
    /// </summary>
    /// <param name="_animations">The animations of the component (name to unique_ptr of Animation or a subclass).</param>
    /// <param name="_current">The animation in use; stopped and replaced when needed.</param>
    /// <param name="_add">Creates one animation: called with the name and the data of a clip.</param>
    /// <param name="_who">Name of the component, for the warnings.</param>
    template <typename Map, typename Current, typename AddFunction>
    void ApplyAnimationClips(const std::vector<AnimationClip>& _clips, const String& _start, std::vector<String>& _appliedClips,
        Map& _animations, Current*& _current, AddFunction _add, const char* _who)
    {
        // Take away what the previous clips made.
        for (const String& _old : _appliedClips)
        {
            const auto _found = _animations.find(_old);
            if (_found == _animations.end()) continue;
            if (_current == _found->second.get())
            {
                _current->Stop();
                _current = nullptr;
            }
            _animations.erase(_found);
        }
        _appliedClips.clear();

        for (const AnimationClip& _clip : _clips)
        {
            if (_clip.frameCount <= 0) continue;
            if (_animations.contains(_clip.name))
            {
                LOG_WARNING(std::string(_who) + ": an animation named \"" + _clip.name + "\" already exists, the clip was not applied");
                continue;
            }
            _add(_clip.name, _clip.ToData());
            _appliedClips.push_back(_clip.name);
        }

        // The animation the level starts with: the chosen one, else the first clip.
        const bool _hasStart = std::find(_appliedClips.begin(), _appliedClips.end(), _start) != _appliedClips.end();
        const String& _first = _appliedClips.empty() ? _start : (_hasStart ? _start : _appliedClips.front());
        const auto _target = _animations.find(_first);
        if (_target != _animations.end())
        {
            if (_current && _current != _target->second.get()) _current->Stop();
            _current = _target->second.get();
        }
    }
}
