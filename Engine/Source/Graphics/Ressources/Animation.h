#pragma once
#include "Graphics/Shapes/ShapeObject.h"
#include "Utilities/System/Timer.h"
#include "Core/KrampusObject.h"

namespace Krampus
{

    enum class ReadDirection
    {
        RD_ROW,
        RD_ROW_REVERSE,
        RD_COLUMN,
        RD_COLUMN_REVERSE,
    };

    struct AnimationData
    {
        Bool                canLoop     = false;
        Int                 count       = 0;
        Float               duration    = 0.0f;
        std::vector<IRect>  sprites;

        AnimationData() = default;
        AnimationData(const Int& _count, const Float& _duration, const IRect& _spriteData,
            const Bool& _canLoop = true, const ReadDirection& _direction = ReadDirection::RD_ROW);

        AnimationData(const Float& _duration, const std::vector<IRect>& _spritesData,
            const Bool& _canLoop = true);
    };

    class Animation : public KrampusObject
    {
    public:
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
        State GetState()
        {
            return state;
        }

        std::unordered_map<Int, Event<>> notifies;

    protected:
        inline Bool IsValidIndex() const
        {
            return currentIndex < data.count;
        }
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
        inline IRect* GetSpriteData()
        {
            if (data.sprites.empty()) return nullptr;

            const int _index = currentIndex == 0 ? 0 : (int)currentIndex - 1;
            return &data.sprites[_index];
        }

    public:
        inline String GetName() const
        {
            return name;
        }


    public:
        Animation(Engine* _engine, const String& _name, ShapeObject* _shape, const AnimationData& _data);
        ~Animation();

    protected:
        void Update();
        void UpdateTimer(const IRect& _spriteData);
        void Reset();

    public:
        void Start();
        void Resume();
        void Pause();
        void Stop();
    };

}

