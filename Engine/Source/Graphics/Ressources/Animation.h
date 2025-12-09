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
        float factor;

        SpriteData(const IVector2& _start, const IVector2& _size, const float& _factor = 1.0f);
        SpriteData(const IRect& _rect, const float& _factor = 1.0f);
    };

    struct AnimationData
    {
        bool canLoop = false;
        bool hasExitTime = false;
        bool isReversed = false;
        int count = 0;
        float duration = 0.0f;
        std::vector<SpriteData> sprites;
        std::unordered_map<unsigned int, std::function<void()>> notifies;
        //std::vector<LinkedAnimation> linkedAnimations;

        AnimationData() = default;
        AnimationData(const int& _count, const float& _duration, const SpriteData& _spriteData,
            const bool& _hasExitTime = true, const bool& _canLoop = true,
            const ReadDirection& _direction = ReadDirection::RD_ROW, const bool _isReversed = false);

        AnimationData(const float& _duration, const std::vector<SpriteData>& _spritesData,
            const bool& _hasExitTime = true, const bool& _canLoop = true,
            const bool& _isReversed = false);
    };

    class Animation
    {
        int currentIndex = 0;
        std::string name;
        AnimationData data;
        ShapeObject* shape;
        Timer* timer;
    
    public:
        Event<> onAnimationEnded;

    private:
        FORCEINLINE bool IsValidIndex() const
        {
            return currentIndex < data.count;
        }
        FORCEINLINE float ComputeDuration()
        {
            return ComputeDuration(*GetSpriteData());
        }
        FORCEINLINE float ComputeDuration(const SpriteData& _spriteData) const
        {
            return data.duration / data.count * _spriteData.factor;
        }
        FORCEINLINE SpriteData* GetSpriteData()
        {
            if (data.sprites.empty()) return nullptr;

            const int _index = currentIndex == 0 ? 0 : currentIndex - 1;
            return &data.sprites[_index];
        }

    public:
        //FORCEINLINE void AddLinkedAnimation(const function<bool()>& _transition, Animation* animation)
        //{
        //    const LinkedAnimation& _linkedAnim = LinkedAnimation(_transition, animation);
        //    data.linkedAnimations.push_back(_linkedAnim);
        //}
        FORCEINLINE std::string GetName() const
        {
            return name;
        }
        //FORCEINLINE Animation* GetNextAnimation() const
        //{
        //    for (const LinkedAnimation& _linkedAnim : data.linkedAnimations)
        //    {
        //        if (_linkedAnim.IsValid()) return _linkedAnim.animation;
        //    }

        //    return nullptr;
        //}

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

