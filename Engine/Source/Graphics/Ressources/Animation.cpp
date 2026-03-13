#include "Animation.h"
#include "Managers/TimerManager.h"
#include "Managers/TextureManager.h"

// AnimationData

Krampus::AnimationData::AnimationData(const Int& _count, const Float& _duration, const IRect& _spriteData, const Bool& _canLoop, const ReadDirection& _direction)
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
                _spriteData.GetPosition().x + _index * _spriteData.GetSize().x,
                _spriteData.GetPosition().y
            };
        },

        [&](const int _index)
        {
            return IVector2
            {
                _spriteData.GetPosition().x - _index * _spriteData.GetSize().x,
                _spriteData.GetPosition().y
            };
        },

        [&](const int _index)
        {
            return IVector2
            {
                _spriteData.GetPosition().x,
                _spriteData.GetPosition().y + _index * _spriteData.GetSize().y
            };
        },

        [&](const int _index)
        {
            return IVector2
            {
                _spriteData.GetPosition().x,
                _spriteData.GetPosition().y - _index * _spriteData.GetSize().y
            };
        }
    };

    for (int _index = 0; _index < _count; _index++)
        sprites.push_back(
            IRect(_computeStart[CAST(int, _direction)](_index), _spriteData.GetSize()
            ));
}

Krampus::AnimationData::AnimationData(const Float& _duration, const std::vector<IRect>& _spritesData, const Bool& _canLoop)
{
    canLoop = _canLoop;
    count = CAST(int, _spritesData.size());
    duration = _duration;
    sprites = _spritesData;
}



// Animation

Krampus::Animation::Animation(Engine* _engine, const String& _name, ShapeObject* _shape, const AnimationData& _data)
    : KrampusObject(_engine)
{
    name = _name;
    shape = _shape;
    data = _data;
    timer = GetWorld()->GetTimerManager()->CreateTimer([&]() { Update(); },
        ComputeDuration(),
        true,
        false);
    state = State::Stoped;
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

    IRect* _spriteData = GetSpriteData();
    if (!_spriteData) return;

    UpdateTimer(*_spriteData);
    M_TEXTURE.SetTextureRect(shape->GetShape(), _spriteData->GetPosition(), _spriteData->GetSize());
}

void Krampus::Animation::UpdateTimer(const IRect& _spriteData)
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
    state = State::Playing;
}

void Krampus::Animation::Resume()
{
    timer->Resume();
    state = State::Playing;
}

void Krampus::Animation::Pause()
{
    timer->Pause();
    state = State::Stoped;
}

void Krampus::Animation::Stop()
{
    Pause();
    Reset();
}