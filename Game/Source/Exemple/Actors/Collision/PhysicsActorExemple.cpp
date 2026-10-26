
#include "PhysicsActorExemple.h"

#include "Components/SpriteComponent.h"

Krampus::Exemple::PhysicsActorExemple::PhysicsActorExemple(Level* _level, bool _isStatic)
    : Krampus::Actor(_level)
{
    if (_isStatic)
    {
        RectangleShapeData _data;
        _data.size = FVector2(1920.0f, 200.0f);
        _data.texturePath = "_internal/KrampusEngineBanner";
        CreateComponent<SpriteComponent>(_data);
        component = CreateComponent<PhysicsComponent>();
        return;
    }

    float _mass = IMath::RandomRange(1.0f, 10.0f);
    bool _isRectangle = IMath::RandomRange(0, 1);
    if (_isRectangle)
    {
        RectangleShapeData _data;
        _data.size = FVector2(75.0f, 35.0f) * _mass / 2.0f;
        _data.texturePath = "_internal/KrampusEngineBanner";
        CreateComponent<SpriteComponent>(_data);
    }
    else
    {
        CircleShapeData _data;
        _data.radius = 35.0f * _mass / 2.0f;
        _data.texturePath = "_internal/KrampusLogo";
        CreateComponent<SpriteComponent>(_data);
    }
    component = CreateComponent<PhysicsComponent>();
}

void Krampus::Exemple::PhysicsActorExemple::SetDynamic()
{
    component->SetDynamic();
}

void Krampus::Exemple::PhysicsActorExemple::SetStatic()
{
    component->SetStatic();
}
