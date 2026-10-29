#include "Animation.h"
#include "Managers/TimerManager.h"
#include "Managers/TextureManager.h"
#include "Managers/LevelManager.h"

// AnimationData

Krampus::AnimationData::AnimationData(const Int& _count, const Float& _duration, const IRect& _spriteData, const Bool& _canLoop, const ReadDirection& _direction, const Int& _wrap)
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

    // On several lines: _wrap frames along the direction, then the next line (down for a row, to the right for a column).
    if (_wrap > 0)
    {
        const bool _byColumn = _direction == ReadDirection::RD_COLUMN || _direction == ReadDirection::RD_COLUMN_REVERSE;
        const bool _reversed = _direction == ReadDirection::RD_ROW_REVERSE || _direction == ReadDirection::RD_COLUMN_REVERSE;
        const IVector2 _origin = _spriteData.GetPosition();
        const IVector2 _size = _spriteData.GetSize();
        for (int _index = 0; _index < _count; _index++)
        {
            const int _along = (_index % _wrap) * (_reversed ? -1 : 1);
            const int _line = _index / _wrap;
            sprites[_index] = _byColumn
                ? IRect(IVector2(_origin.x + _line * _size.x, _origin.y + _along * _size.y), _size)
                : IRect(IVector2(_origin.x + _along * _size.x, _origin.y + _line * _size.y), _size);
        }
    }
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
    Level* _level = GetWorld()->GetLevelManager()->GetCurrentLevel();
    if (!_level) { LOG_ERROR("Animation \"" + _name + "\": there is no level to run the animation in"); state = State::Stoped; return; }
    timer = _level->GetTimerManager().CreateTimer([&]() { Update(); },
        ComputeDuration(),
        true,
        false);
    state = State::Stoped;
}

Krampus::Animation::~Animation()
{
    if (timer) timer->Stop();
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
    if (!timer) return;
    timer->SetDuration(_duration);
    if (timer) timer->Reset();
}

void Krampus::Animation::Reset()
{
    currentIndex = 0;
    if (timer) timer->Reset();
}

void Krampus::Animation::Start()
{
    LOG(VerbosityType::Verbose, "Animation: start \"" + name.StdString() + "\" (" + std::to_string(static_cast<int>(data.count)) + " frames)");
    Update();
    if (timer) timer->Play();
    state = State::Playing;
}

void Krampus::Animation::Resume()
{
    if (timer) timer->Resume();
    state = State::Playing;
}

void Krampus::Animation::Pause()
{
    LOG(VerbosityType::VeryVerbose, "Animation: pause \"" + name.StdString() + "\"");
    if (timer) timer->Pause();
    state = State::Stoped;
}

void Krampus::Animation::Stop()
{
    LOG(VerbosityType::Verbose, "Animation: stop \"" + name.StdString() + "\"");
    Pause();
    Reset();
}