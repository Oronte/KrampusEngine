
#include "PhysicsComponent.h"
#include "GameFramework/Level.h"

Krampus::PhysicsComponent::PhysicsComponent(Actor* _owner)
    : Component(_owner)
{
    Init();

    SpriteComponent* _sprite = _owner->GetComponent<SpriteComponent>();
    if (!_sprite)
    {
        LOG_ERROR("Sprite component not found");
        return;
    }

    if (_sprite->GetShapeType() == ShapeType::Circle)
        MakeCircle(_sprite->GetRadius());
    else if (_sprite->GetShapeType() == ShapeType::Rectangle)
        MakeRectangle(_sprite->GetSize());
}

Krampus::PhysicsComponent::PhysicsComponent(Actor* _owner, const Float& _radius)
    : Component(_owner)
{
    Init();
    MakeCircle(_radius);
}

Krampus::PhysicsComponent::PhysicsComponent(Actor* _owner, const FVector2& _size)
    : Component(_owner)
{
    Init();
    MakeRectangle(_size);
}

void Krampus::PhysicsComponent::Deconstruct()
{
	Super::Deconstruct();
	b2DestroyBody(id);
}

void Krampus::PhysicsComponent::OnCollisionResolve()
{
    if (body.type != b2_dynamicBody)
        return;

    b2Vec2 _pos = b2Body_GetPosition(id);
    FVector2 _sfPos = FVector2(_pos.x, _pos.y) * PX_TO_M;
    float _angle = b2Rot_GetAngle(b2Body_GetRotation(id));

    SetPosition(_sfPos);
    SetRotation(Angle(_angle));
}

void Krampus::PhysicsComponent::SetDynamic()
{
    b2Body_SetType(id, b2BodyType::b2_dynamicBody);
	body.type = b2BodyType::b2_dynamicBody;
}

void Krampus::PhysicsComponent::SetStatic()
{
    b2Body_SetType(id, b2BodyType::b2_staticBody);
	body.type = b2BodyType::b2_staticBody;
}

void Krampus::PhysicsComponent::SetKinematic()
{
    b2Body_SetType(id, b2BodyType::b2_kinematicBody);
	body.type = b2BodyType::b2_kinematicBody;
}

void Krampus::PhysicsComponent::Init()
{
    body = b2DefaultBodyDef();
	body.type = b2BodyType::b2_dynamicBody;
    const FVector2 _pos = GetPosition() / PX_TO_M;
	body.position = b2Vec2(_pos.x, _pos.y);
	body.rotation = b2MakeRot(GetRotation());

    shape = b2DefaultShapeDef();
	shape.density = 1.0f;

    id = b2CreateBody(GetLevel()->GetPhysicsWorldID(), &body);

    if (!b2Body_IsValid(id))
        LOG_ERROR("The body id is Invalid !");

     collisionResolveHandle = GetLevel()->GetCollisionManagerRef().onResolveCollision.AddListener(this, &PhysicsComponent::OnCollisionResolve);
     moveHandle = GetOwner()->onMove.AddListener([this] (FVector2 _pos)
     {
         FVector2 _meterPos = _pos / PX_TO_M;
         b2Body_SetTransform(id, b2Vec2(_meterPos.x, _meterPos.y), b2MakeRot(GetRotation()));
     });
     rotateHandle = GetOwner()->onRotate.AddListener([this] (Angle _rot)
     {
         FVector2 _meterPos = GetPosition() / PX_TO_M;
         b2Body_SetTransform(id, b2Vec2(_meterPos.x, _meterPos.y), b2MakeRot(_rot));
     });
}

void Krampus::PhysicsComponent::MakeCircle(const Float& _radius)
{
    b2Circle _circle = {0};
    _circle.radius = _radius / PX_TO_M;
    b2CreateCircleShape(id, &shape, &_circle);
}

void Krampus::PhysicsComponent::MakeRectangle(const FVector2& _size)
{
    b2Polygon _polygon = b2MakeBox(_size.x / PX_TO_M / 2.0f, _size.y / PX_TO_M / 2.0f);
    b2CreatePolygonShape(id, &shape, &_polygon);
}
