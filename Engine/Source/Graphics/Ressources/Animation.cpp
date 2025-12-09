#include "Animation.h"
#include "Managers/TimerManager.h"
#include "Managers/TextureManager.h"

// SpriteData

Krampus::SpriteData::SpriteData(const IVector2& _start, const IVector2& _size, const float& _factor)
{
    start = _start;
    size = _size;
    factor = _factor;
}

Krampus::SpriteData::SpriteData(const IRect& _rect, const float& _factor)
{
    start = _rect.GetPosition();
    size = _rect.GetSize();
    factor = _factor;
}



// AnimationData

Krampus::AnimationData::AnimationData(const int& _count, const float& _duration, const SpriteData& _spriteData, const bool& _hasExitTime, const bool& _canLoop, const ReadDirection& _direction, const bool _isReversed)
{
    canLoop = _canLoop;
    hasExitTime = _hasExitTime;
    isReversed = _isReversed;
    count = _count;
    duration = _duration;

    // Callbacks for the ReadDirection // TODO Change
    const std::function<IVector2(const int _index)> _computeStart[] =
    {
        [&](const int _index)
        {
            return IVector2
            {
                _spriteData.start.x + _index * _spriteData.size.x,
                _spriteData.start.y
            };
        },

        [&](const int _index)
        {
            return IVector2
            {
                _spriteData.start.x - _index * _spriteData.size.x,
                _spriteData.start.y
            };
        },

        [&](const int _index)
        {
            return IVector2
            {
                _spriteData.start.x,
                _spriteData.start.y + _index * _spriteData.size.y
            };
        },

        [&](const int _index)
        {
            return IVector2
            {
                _spriteData.start.x,
                _spriteData.start.y - _index * _spriteData.size.y
            };
        }
    };

    for (int _index = 0; _index < _count; _index++)
    {
        const SpriteData& _data = SpriteData(_computeStart[(int)_direction](_index), _spriteData.size, _spriteData.factor);
        sprites.push_back(_data);
    }
}

Krampus::AnimationData::AnimationData(const float& _duration, const std::vector<SpriteData>& _spritesData, const bool& _hasExitTime, const bool& _canLoop, const bool& _isReversed)
{
    canLoop = _canLoop;
    hasExitTime = _hasExitTime;
    isReversed = _isReversed;
    count = CAST(int, _spritesData.size());
    duration = _duration;
    sprites = _spritesData;
}



// Animation

Krampus::Animation::Animation(const std::string& _name, ShapeObject* _shape, const AnimationData& _data)
{
    name = _name;
    shape = _shape;
    data = _data;
    timer = M_TIMER.CreateTimer([&]() { Update(); },
        ComputeDuration(),
        false,
        true);
}

Krampus::Animation::~Animation()
{
    timer->Stop();
}

void Krampus::Animation::Update()
{
    if (!IsValidIndex())
    {
        if (!data.canLoop)
        {
            onAnimationEnded();
            Stop();
            return;
        }

        //TODO remove
        onAnimationEnded.Broadcast();
        Reset();
    }

    if (data.isReversed)
    {
        shape->GetShape()->SetScale(FVector2(-1.0f, 1.0f));
    }

    ++currentIndex;

    std::unordered_map<unsigned int, std::function<void()>> _notifies = data.notifies;
    if (_notifies.contains(currentIndex))
    {
        _notifies[currentIndex]();
    }

    const SpriteData& _spriteData = *GetSpriteData();
    UpdateTimer(_spriteData);
    M_TEXTURE.SetTextureRect(shape->GetShape(), _spriteData.start, _spriteData.size);
}

void Krampus::Animation::UpdateTimer(const SpriteData& _spriteData)
{
    const float& _duration = ComputeDuration(_spriteData);
    timer->SetDuration(_duration);
    timer->Reset();
}

void Krampus::Animation::Reset()
{
    currentIndex = 0;
    timer->Reset();
}

void Krampus::Animation::Start()
{
    Update();
    timer->Start();
}

void Krampus::Animation::Resume()
{
    timer->Resume();
}

void Krampus::Animation::Pause()
{
    timer->Pause();
}

void Krampus::Animation::Stop()
{
    Pause();
    Reset();
}