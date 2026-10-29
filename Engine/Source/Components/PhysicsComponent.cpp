
#include "PhysicsComponent.h"
#include "GameFramework/Level.h"
#include "Utilities/Debug/Debug.h"

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

	// The body is gone after this: nothing may reach it anymore (the physics step of this frame still broadcasts, and the actor
	// can still move until it is deleted at the end of the frame).
	collisionResolveHandle.Disconnect();
	moveHandle.Disconnect();
	rotateHandle.Disconnect();

	if (b2Body_IsValid(bodyId))
		b2DestroyBody(bodyId);
	bodyId = b2_nullBodyId;
	shapeId = b2_nullShapeId;
}

void Krampus::PhysicsComponent::OnCollisionResolve()
{
    if (!b2Body_IsValid(bodyId) || b2Body_GetType(bodyId) != b2_dynamicBody)
        return;

    b2Vec2 _pos = b2Body_GetPosition(bodyId);
    FVector2 _sfPos = FVector2(_pos.x, _pos.y) * PX_TO_M;
    float _angle = b2Rot_GetAngle(b2Body_GetRotation(bodyId));

    SetPosition(_sfPos);
    SetRotation(Angle(_angle));
}

void Krampus::PhysicsComponent::SetDynamic()
{
    b2Body_SetType(bodyId, b2BodyType::b2_dynamicBody);
}

void Krampus::PhysicsComponent::SetStatic()
{
    b2Body_SetType(bodyId, b2BodyType::b2_staticBody);
}

void Krampus::PhysicsComponent::SetKinematic()
{
    b2Body_SetType(bodyId, b2BodyType::b2_kinematicBody);
}

void Krampus::PhysicsComponent::SetLinearVelocity(const FVector2& _velocity)
{
	b2Body_SetLinearVelocity(bodyId, b2Vec2(_velocity.x / PX_TO_M, _velocity.y / PX_TO_M));
}

Krampus::FVector2 Krampus::PhysicsComponent::GetLinearVelocity() const
{
	b2Vec2 _velocity = b2Body_GetLinearVelocity(bodyId);
	return FVector2(_velocity.x * PX_TO_M, _velocity.y * PX_TO_M);
}

void Krampus::PhysicsComponent::SetAngularVelocity(const Float& _velocity)
{
	b2Body_SetAngularVelocity(bodyId, _velocity / PX_TO_M);
}

Float Krampus::PhysicsComponent::GetAngularVelocity() const
{
	return b2Body_GetAngularVelocity(bodyId) * PX_TO_M;
}

void Krampus::PhysicsComponent::SetLinearDamping(const Float& _damping)
{
	b2Body_SetLinearDamping(bodyId, _damping);
}

Float Krampus::PhysicsComponent::GetLinearDamping() const
{
	return b2Body_GetLinearDamping(bodyId);
}

void Krampus::PhysicsComponent::SetAngularDamping(const Float& _damping)
{
	b2Body_SetAngularDamping(bodyId, _damping);
}

Float Krampus::PhysicsComponent::GetAngularDamping() const
{
	return b2Body_GetAngularDamping(bodyId);
}

void Krampus::PhysicsComponent::SetGravityScale(const Float& _scale)
{
	b2Body_SetGravityScale(bodyId, _scale);
}

Float Krampus::PhysicsComponent::GetGravityScale() const
{
	return b2Body_GetGravityScale(bodyId);
}

void Krampus::PhysicsComponent::SetMassData(const Float& _mass, const Float& _inertia, const FVector2& _center)
{
	b2MassData _data;
	_data.center = b2Vec2(_center.x / PX_TO_M, _center.y / PX_TO_M);
	_data.mass = _mass / PX_TO_M;
	_data.rotationalInertia = _inertia;
	b2Body_SetMassData(bodyId, _data);
}

void Krampus::PhysicsComponent::GetMassData(Float& _mass, Float& _inertia, FVector2& _center) const
{
	b2MassData _data = b2Body_GetMassData(bodyId);
	_mass = _data.mass * PX_TO_M;
	_inertia = _data.rotationalInertia;
	_center = FVector2(_data.center.x * PX_TO_M, _data.center.y * PX_TO_M);
}

void Krampus::PhysicsComponent::ApplyMassFromShapes()
{
	b2Body_ApplyMassFromShapes(bodyId);
}

void Krampus::PhysicsComponent::SetFixedRotation(const bool& _fixed)
{
	b2Body_SetFixedRotation(bodyId, _fixed);
}

void Krampus::PhysicsComponent::EnableSleeping(const bool& _enable)
{
	b2Body_EnableSleep(bodyId, _enable);
}

bool Krampus::PhysicsComponent::IsSleepingAllowed() const
{
	return b2Body_IsSleepEnabled(bodyId);
}

void Krampus::PhysicsComponent::SetSleepThreshold(const Float& _threshold)
{
	b2Body_SetSleepThreshold(bodyId, _threshold);
}

Float Krampus::PhysicsComponent::GetSleepThreshold() const
{
	return b2Body_GetSleepThreshold(bodyId);
}

void Krampus::PhysicsComponent::SetAwake(const bool& _awake)
{
	b2Body_SetAwake(bodyId, _awake);
}

bool Krampus::PhysicsComponent::IsAwake() const
{
	return b2Body_IsAwake(bodyId);
}

void Krampus::PhysicsComponent::Enable()
{
	b2Body_Enable(bodyId);
}

void Krampus::PhysicsComponent::Disable()
{
	b2Body_Disable(bodyId);
}

void Krampus::PhysicsComponent::EnableContactEvents(bool _enable)
{
	b2Body_EnableContactEvents(bodyId, _enable);
}

void Krampus::PhysicsComponent::SetBullet(const bool& _bullet)
{
	b2Body_SetBullet(bodyId, _bullet);
}

void Krampus::PhysicsComponent::SetFilter(const CollisionChannel& _category, const CollisionChannel& _mask, int32_t groupIndex)
{
    b2Shape_SetFilter(shapeId, MakeFilter(_category, _mask, groupIndex));
}

void Krampus::PhysicsComponent::Init()
{
    b2BodyDef _body = b2DefaultBodyDef();
    _body.type = b2BodyType::b2_dynamicBody;
    const FVector2 _pos = GetPosition() / PX_TO_M;
    _body.position = b2Vec2(_pos.x, _pos.y);
    _body.rotation = b2MakeRot(GetRotation());

    bodyId = b2CreateBody(GetLevel()->GetWorldPhysics().GetPhysicsWorldID(), &_body);

    if (!b2Body_IsValid(bodyId))
    {
        LOG_ERROR("PhysicsComponent: box2d could not create the body of \"" + GetOwner()->GetName().StdString() + "\"");
        return;		// nothing below makes sense without a body
    }

     collisionResolveHandle = GetLevel()->GetWorldPhysics().onResolveCollision.AddListener(this, &PhysicsComponent::OnCollisionResolve);
     moveHandle = GetOwner()->onMove.AddListener([this] (FVector2 _pos)
     {
         FVector2 _meterPos = _pos / PX_TO_M;
         b2Body_SetTransform(bodyId, b2Vec2(_meterPos.x, _meterPos.y), b2MakeRot(GetRotation()));
     });
     rotateHandle = GetOwner()->onRotate.AddListener([this] (Angle _rot)
     {
         FVector2 _meterPos = GetPosition() / PX_TO_M;
         b2Body_SetTransform(bodyId, b2Vec2(_meterPos.x, _meterPos.y), b2MakeRot(_rot));
     });
}

void Krampus::PhysicsComponent::MakeCircle(const Float& _radius)
{
    isCircle = true;
    debugRadius = _radius;
    b2Circle _circle = {0};
    _circle.radius = _radius / PX_TO_M;
    b2ShapeDef _shape = b2DefaultShapeDef();
    _shape.density = 1.0f;
    shapeId = b2CreateCircleShape(bodyId, &_shape, &_circle);
}

void Krampus::PhysicsComponent::MakeRectangle(const FVector2& _size)
{
    isCircle = false;
    debugSize = _size;
    b2Polygon _polygon = b2MakeBox(_size.x / PX_TO_M / 2.0f, _size.y / PX_TO_M / 2.0f);
    b2ShapeDef _shape = b2DefaultShapeDef();
    _shape.density = 1.0f;
    shapeId = b2CreatePolygonShape(bodyId, &_shape, &_polygon);
}


int Krampus::PhysicsComponent::GetBodyType() const
{
    return static_cast<int>(b2Body_GetType(bodyId));
}

void Krampus::PhysicsComponent::SetBodyType(const int& _type)
{
    b2Body_SetType(bodyId, static_cast<b2BodyType>(_type));
}

bool Krampus::PhysicsComponent::IsFixedRotation() const
{
	return b2Body_IsFixedRotation(bodyId);
}

bool Krampus::PhysicsComponent::IsEnabled() const
{
	return b2Body_IsEnabled(bodyId);
}

bool Krampus::PhysicsComponent::AreContactEventsEnabled() const
{
	return b2Shape_IsValid(shapeId) && b2Shape_AreContactEventsEnabled(shapeId);
}

bool Krampus::PhysicsComponent::IsBullet() const
{
	return b2Body_IsBullet(bodyId);
}

void Krampus::PhysicsComponent::GetFilter(CollisionChannel& _category, CollisionChannel& _mask, int32_t& _groupIndex) const
{
	const b2Filter _filter = b2Shape_GetFilter(shapeId);
	_category = static_cast<CollisionChannel>(_filter.categoryBits);
	_mask = static_cast<CollisionChannel>(_filter.maskBits);
	_groupIndex = _filter.groupIndex;
}

void Krampus::PhysicsComponent::DrawDebug()
{
#ifdef DEBUG
	if (!b2Body_IsValid(bodyId)) return;

	// The shape where box2d has it (it is what collides), not where the actor is drawn.
	const b2Vec2 _position = b2Body_GetPosition(bodyId);
	const FVector2 _center = FVector2(_position.x, _position.y) * PX_TO_M;
	const Angle _rotation = Angle(b2Rot_GetAngle(b2Body_GetRotation(bodyId)));

	if (isCircle)
		Debug::DrawDebugCircle(GetLevel(), _center, debugRadius, 20, Color::Cyan());
	else
		Debug::DrawDebugRect(GetLevel(), _center, debugSize, _rotation, Color::Cyan());
#endif
}
