
#include "PhysicsComponent.h"
#include "GameFramework/Level.h"

Krampus::PhysicsComponent::PhysicsComponent(Actor* _owner)
    : Component(_owner)
{
	name = NAME_OF(PhysicsComponent);

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
    name = NAME_OF(PhysicsComponent);
    Init();
    MakeCircle(_radius);
}

Krampus::PhysicsComponent::PhysicsComponent(Actor* _owner, const FVector2& _size)
    : Component(_owner)
{
	name = NAME_OF(PhysicsComponent);
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
    if (b2Body_GetType(id) != b2_dynamicBody)
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
}

void Krampus::PhysicsComponent::SetStatic()
{
    b2Body_SetType(id, b2BodyType::b2_staticBody);
}

void Krampus::PhysicsComponent::SetKinematic()
{
    b2Body_SetType(id, b2BodyType::b2_kinematicBody);
}

void Krampus::PhysicsComponent::SetLinearVelocity(const FVector2& _velocity)
{
	b2Body_SetLinearVelocity(id, b2Vec2(_velocity.x / PX_TO_M, _velocity.y / PX_TO_M));
}

Krampus::FVector2 Krampus::PhysicsComponent::GetLinearVelocity() const
{
	b2Vec2 _velocity = b2Body_GetLinearVelocity(id);
	return FVector2(_velocity.x * PX_TO_M, _velocity.y * PX_TO_M);
}

void Krampus::PhysicsComponent::SetAngularVelocity(const Float& _velocity)
{
	b2Body_SetAngularVelocity(id, _velocity / PX_TO_M);
}

Float Krampus::PhysicsComponent::GetAngularVelocity() const
{
	return b2Body_GetAngularVelocity(id) * PX_TO_M;
}

void Krampus::PhysicsComponent::SetLinearDamping(const Float& _damping)
{
	b2Body_SetLinearDamping(id, _damping);
}

Float Krampus::PhysicsComponent::GetLinearDamping() const
{
	return b2Body_GetLinearDamping(id);
}

void Krampus::PhysicsComponent::SetAngularDamping(const Float& _damping)
{
	b2Body_SetAngularDamping(id, _damping);
}

Float Krampus::PhysicsComponent::GetAngularDamping() const
{
	return b2Body_GetAngularDamping(id);
}

void Krampus::PhysicsComponent::SetGravityScale(const Float& _scale)
{
	b2Body_SetGravityScale(id, _scale);
}

Float Krampus::PhysicsComponent::GetGravityScale() const
{
	return b2Body_GetGravityScale(id);
}

void Krampus::PhysicsComponent::SetMassData(const Float& _mass, const Float& _inertia, const FVector2& _center)
{
	b2MassData _data;
	_data.center = b2Vec2(_center.x / PX_TO_M, _center.y / PX_TO_M);
	_data.mass = _mass / PX_TO_M;
	_data.rotationalInertia = _inertia;
	b2Body_SetMassData(id, _data);
}

void Krampus::PhysicsComponent::GetMassData(Float& _mass, Float& _inertia, FVector2& _center) const
{
	b2MassData _data = b2Body_GetMassData(id);
	_mass = _data.mass;
	_inertia = _data.rotationalInertia;
	_center = FVector2(_data.center.x * PX_TO_M, _data.center.y * PX_TO_M);
}

void Krampus::PhysicsComponent::ApplyMassFromShapes()
{
	b2Body_ApplyMassFromShapes(id);
}

void Krampus::PhysicsComponent::SetFixedRotation(const bool& _fixed)
{
	b2Body_SetFixedRotation(id, _fixed);
}

void Krampus::PhysicsComponent::EnableSleeping(const bool& _enable)
{
	b2Body_EnableSleep(id, _enable);
}

bool Krampus::PhysicsComponent::IsSleepingAllowed() const
{
	return b2Body_IsSleepEnabled(id);
}

void Krampus::PhysicsComponent::SetSleepThreshold(const Float& _threshold)
{
	b2Body_SetSleepThreshold(id, _threshold);
}

Float Krampus::PhysicsComponent::GetSleepThreshold() const
{
	return b2Body_GetSleepThreshold(id);
}

void Krampus::PhysicsComponent::SetAwake(const bool& _awake)
{
	b2Body_SetAwake(id, _awake);
}

bool Krampus::PhysicsComponent::IsAwake() const
{
	return b2Body_IsAwake(id);
}

void Krampus::PhysicsComponent::Enable()
{
	b2Body_Enable(id);
}

void Krampus::PhysicsComponent::Disable()
{
	b2Body_Disable(id);
}

void Krampus::PhysicsComponent::EnableContactEvents(bool _enable)
{
	b2Body_EnableContactEvents(id, _enable);
}

void Krampus::PhysicsComponent::SetBullet(const bool& _bullet)
{
	b2Body_SetBullet(id, _bullet);
}

void Krampus::PhysicsComponent::Init()
{
    b2BodyDef _body = b2DefaultBodyDef();
    _body.type = b2BodyType::b2_dynamicBody;
    const FVector2 _pos = GetPosition() / PX_TO_M;
    _body.position = b2Vec2(_pos.x, _pos.y);
    _body.rotation = b2MakeRot(GetRotation());

    id = b2CreateBody(GetLevel()->GetWorldPhysics().GetPhysicsWorldID(), &_body);

    if (!b2Body_IsValid(id))
        LOG_ERROR("The body id is Invalid !");

     collisionResolveHandle = GetLevel()->GetWorldPhysics().onResolveCollision.AddListener(this, &PhysicsComponent::OnCollisionResolve);
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
    b2ShapeDef _shape = b2DefaultShapeDef();
    _shape.density = 1.0f;
    b2CreateCircleShape(id, &_shape, &_circle);
}

void Krampus::PhysicsComponent::MakeRectangle(const FVector2& _size)
{
    b2Polygon _polygon = b2MakeBox(_size.x / PX_TO_M / 2.0f, _size.y / PX_TO_M / 2.0f);
    b2ShapeDef _shape = b2DefaultShapeDef();
    _shape.density = 1.0f;
    b2CreatePolygonShape(id, &_shape, &_polygon);
}
