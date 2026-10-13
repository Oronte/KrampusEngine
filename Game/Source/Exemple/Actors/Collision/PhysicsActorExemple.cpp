
#include "PhysicsActorExemple.h"

#include "Components/SpriteComponent.h"

Krampus::Exemple::PhysicsActorExemple::PhysicsActorExemple(Level* _level)
    : Krampus::Actor(_level)
{
    float _mass = IMath::RandomRange(1.0f, 10.0f);
    bool _isRectangle = IMath::RandomRange(0, 1);
    if (_isRectangle)
    {
        RectangleShapeData _data;
        _data.size = FVector2(100.0f, 50.0f) * _mass / 2.0f;
        _data.texturePath = "_internal/KrampusEngineBanner";
        CreateComponent<SpriteComponent>(_data);
    }
    else
    {
        CircleShapeData _data;
        _data.radius = 20.0f * _mass / 2.0f;
        _data.texturePath = "_internal/KrampusLogo";
        CreateComponent<SpriteComponent>(_data);
    }
    component = CreateComponent<PhysicsComponent>();
    SetStatic();
}

void Krampus::Exemple::PhysicsActorExemple::SetDynamic()
{
    component->SetDynamic();
}

void Krampus::Exemple::PhysicsActorExemple::SetStatic()
{
    component->SetStatic();
}
