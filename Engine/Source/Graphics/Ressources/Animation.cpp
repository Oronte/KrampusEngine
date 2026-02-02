#include "Animation.h"
#include "Managers/TimerManager.h"
#include "Managers/TextureManager.h"

// SpriteData

Krampus::SpriteData::SpriteData(const IVector2& _start, const IVector2& _size)
{
    start = _start;
    size = _size;
}

Krampus::SpriteData::SpriteData(const IRect& _rect)
{
    start = _rect.GetPosition();
    size = _rect.GetSize();
}



// AnimationData

Krampus::AnimationData::AnimationData(const int& _count, const float& _duration, const SpriteData& _spriteData, const bool& _canLoop, const ReadDirection& _direction)
{
    canLoop = _canLoop;
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
        sprites.push_back(
            SpriteData(_computeStart[CAST(int, _direction)](_index), _spriteData.size
            ));
}

Krampus::AnimationData::AnimationData(const float& _duration, const std::vector<SpriteData>& _spritesData, const bool& _canLoop)
{
    canLoop = _canLoop;
    count = CAST(int, _spritesData.size());
    duration = _duration;
    sprites = _spritesData;
}



// Animation

Krampus::Animation::Animation(Engine* _engine, const std::string& _name, ShapeObject* _shape, const AnimationData& _data)
    : KrampusObject(_engine)
{
    name = _name;
    shape = _shape;
    data = _data;
    timer = GetWorld()->GetTimerManager()->CreateTimer([&]() { Update(); },
        ComputeDuration(),
        true,
        false);
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
            Stop();
            return;
        }

        Reset();
    }

    notifies[currentIndex].Broadcast();

    ++currentIndex;

    SpriteData* _spriteData = GetSpriteData();
    if (!_spriteData) return;

    UpdateTimer(*_spriteData);
    M_TEXTURE.SetTextureRect(shape->GetShape(), _spriteData->start, _spriteData->size);
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
    timer->Play();
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