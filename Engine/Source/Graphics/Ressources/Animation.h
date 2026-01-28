#pragma once
#include "Graphics/Shapes/ShapeObject.h"
#include "Utilities/System/Timer.h"

namespace Krampus
{

    enum class ReadDirection
    {
        RD_ROW,
        RD_ROW_REVERSE,
        RD_COLUMN,
        RD_COLUMN_REVERSE,
    };

    struct SpriteData
    {
        IVector2 start;
        IVector2 size;

        SpriteData(const IVector2& _start, const IVector2& _size);
        SpriteData(const IRect& _rect);
    };

    struct AnimationData
    {
        bool canLoop = false;
        int count = 0;
        float duration = 0.0f;
        std::vector<SpriteData> sprites;

        AnimationData() = default;
        AnimationData(const int& _count, const float& _duration, const SpriteData& _spriteData,
            const bool& _canLoop = true, const ReadDirection& _direction = ReadDirection::RD_ROW);

        AnimationData(const float& _duration, const std::vector<SpriteData>& _spritesData,
            const bool& _canLoop = true);
    };

    class Animation
    {
        int currentIndex = 0;
        std::string name;
        AnimationData data;
        ShapeObject* shape;
        Timer* timer;

    public:
        std::unordered_map<int, Event<>> notifies;

    private:
        inline bool IsValidIndex() const
        {
            return currentIndex < data.count;
        }
        inline float ComputeDuration()
        {
            SpriteData* _spriteData = GetSpriteData();
            if (!_spriteData)
            {
                LOG_ERROR("There is no sprite data to Compute the duration");
                return 0.0f;
            }
            return ComputeDuration(*_spriteData);
        }
        inline float ComputeDuration(const SpriteData& _spriteData) const
        {
            const int& _count = data.count;
            if (_count == 0)
            {
                LOG_ERROR("Can't Compute the duration if sprite count == 0");
                return 0.0f;
            }
            return data.duration / CAST(float, _count);
        }
        inline SpriteData* GetSpriteData()
        {
            if (data.sprites.empty()) return nullptr;

            const int _index = currentIndex == 0 ? 0 : currentIndex - 1;
            return &data.sprites[_index];
        }

    public:
        inline std::string GetName() const
        {
            return name;
        }


    public:
        Animation(const std::string& _name, ShapeObject* _shape, const AnimationData& _data);
        ~Animation();

    private:
        void Update();
        void UpdateTimer(const SpriteData& _spriteData);
        void Reset();

    public:
        void Start();
        void Resume();
        void Pause();
        void Stop();
    };

}

