
#include "PhysicsComponent.h"
#include "GameFramework/Level.h"
#include "Utilities/Debug/Debug.h"

Krampus::PhysicsComponent::PhysicsComponent(Actor* _owner)
    : Component(_owner)
{
	name = NAME_OF(PhysicsComponent);

    Init();

    // The shape of the sprite, now and when it changes (Tick). Without a sprite: a box until one is added.
    if (!_owner->GetComponent<SpriteComponent>())
        LOG_WARNING("PhysicsComponent: \"" + _owner->GetName().StdString() + "\" has no SpriteComponent yet: its body is a box of 100 x 100 until it gets one");
    MakeRectangle(FVector2(100.f, 100.f));
    SyncWithSprite();
}

Krampus::PhysicsComponent::PhysicsComponent(Actor* _owner, const Float& _radius)
    : Component(_owner)
{
    name = NAME_OF(PhysicsComponent);
    matchSprite = false;
    Init();
    MakeCircle(_radius);
}

Krampus::PhysicsComponent::PhysicsComponent(Actor* _owner, const FVector2& _size)
    : Component(_owner)
{
	name = NAME_OF(PhysicsComponent);
    matchSprite = false;
    Init();
    MakeRectangle(_size);
}

void Krampus::PhysicsComponent::Construct()
{
    Super::Construct();

    FVector2 _meterPos = GetPosition() / PX_TO_M;
    b2Body_SetTransform(bodyId, b2Vec2(_meterPos.x, _meterPos.y), b2MakeRot(GetRotation()));
    SyncWithSprite();
}

void Krampus::PhysicsComponent::Tick(const Float& _deltaTime)
{
    Super::Tick(_deltaTime);
    SyncWithSprite();
}

void Krampus::PhysicsComponent::Deconstruct()
{
	Super::Deconstruct();

	// The body is gone after this: nothing may reach it anymore (the physics step of this frame still broadcasts, and the actor
	// can still move until it is deleted at the end of the frame).
	collisionResolveHandle.Disconnect();
	moveHandle.Disconnect();
	rotateHandle.Disconnect();
	scaleHandle.Disconnect();

	shapeSavedHandle.Disconnect();

	if (b2Body_IsValid(bodyId))
		b2DestroyBody(bodyId);
	bodyId = b2_nullBodyId;
	shapeId = b2_nullShapeId;
	shapeIds.clear();
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
    filter = MakeFilter(_category, _mask, groupIndex);
    for (const b2ShapeId _shape : shapeIds)		// every piece of a concave polygon
        if (b2Shape_IsValid(_shape)) b2Shape_SetFilter(_shape, filter);
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

     // The body is scaled like the actor (and its sprite).
     scaleHandle = GetOwner()->onScale.AddListener([this](FVector2 _scale)
     {
         if (_scale != builtScale) Rebuild();
     });

     // A shape saved in the Shape Editor is rebuilt at once.
     shapeSavedHandle = ShapeAssets::OnChanged().AddListener([this](const std::string& _name)
     {
         if (shapeType == ShapeType::Polygon && _name == polygonName) MakePolygon(polygonName);
     });
}

void Krampus::PhysicsComponent::SetMatchSprite(const bool& _match)
{
    matchSprite = _match;
    SyncWithSprite();
}

void Krampus::PhysicsComponent::SyncWithSprite()
{
    if (!b2Body_IsValid(bodyId)) return;
    if (GetScale() != builtScale) Rebuild();
    if (!matchSprite) return;

    SpriteComponent* _sprite = GetOwner()->GetComponent<SpriteComponent>();
    if (!_sprite) return;

    // Rebuilt only when something differs: the box2d shapes are not made again every frame.
    switch (_sprite->GetShapeType())
    {
    case ShapeType::Circle:
        if (shapeType != ShapeType::Circle || static_cast<float>(debugRadius) != static_cast<float>(_sprite->GetRadius()))
            MakeCircle(_sprite->GetRadius());
        break;
    case ShapeType::Rectangle:
        if (shapeType != ShapeType::Rectangle || debugSize != _sprite->GetSize())
            MakeRectangle(_sprite->GetSize());
        break;
    case ShapeType::Polygon:
        if (shapeType != ShapeType::Polygon || polygonName != _sprite->GetShapeObject()->GetPolygonName())
            MakePolygon(_sprite->GetShapeObject()->GetPolygonName());
        break;
    }
}

void Krampus::PhysicsComponent::Rebuild()
{
    if (shapeType == ShapeType::Circle) MakeCircle(debugRadius);
    else if (shapeType == ShapeType::Rectangle) MakeRectangle(debugSize);
    else MakePolygon(polygonName);
}

void Krampus::PhysicsComponent::DestroyShapes()
{
    for (const b2ShapeId _shape : shapeIds)
        if (b2Shape_IsValid(_shape)) b2DestroyShape(_shape, false);
    shapeIds.clear();
    shapeId = b2_nullShapeId;
    polygonParts.clear();
}

void Krampus::PhysicsComponent::AddShape(const b2ShapeId _shape)
{
    if (!b2Shape_IsValid(_shape)) return;
    b2Shape_SetFilter(_shape, filter);
    if (shapeIds.empty()) shapeId = _shape;
    shapeIds.push_back(_shape);
}

void Krampus::PhysicsComponent::MakeCircle(const Float& _radius)
{
    if (!b2Body_IsValid(bodyId)) return;
    DestroyShapes();
    shapeType = ShapeType::Circle;
    debugRadius = _radius;
    builtScale = GetScale();
    b2Circle _circle = {0};
    const float _scale = std::max(std::abs(builtScale.x), std::abs(builtScale.y));	// like the sprite's outline
    _circle.radius = std::max(0.5f, static_cast<float>(_radius) * _scale) / PX_TO_M;
    b2ShapeDef _shape = b2DefaultShapeDef();
    _shape.density = 1.0f;
    AddShape(b2CreateCircleShape(bodyId, &_shape, &_circle));
    b2Body_ApplyMassFromShapes(bodyId);
}

void Krampus::PhysicsComponent::MakeRectangle(const FVector2& _size)
{
    if (!b2Body_IsValid(bodyId)) return;
    DestroyShapes();
    shapeType = ShapeType::Rectangle;
    debugSize = _size;
    builtScale = GetScale();
    const FVector2 _scaled = FVector2(std::max(1.f, _size.x * std::abs(builtScale.x)), std::max(1.f, _size.y * std::abs(builtScale.y)));
    b2Polygon _polygon = b2MakeBox(_scaled.x / PX_TO_M / 2.0f, _scaled.y / PX_TO_M / 2.0f);
    b2ShapeDef _shape = b2DefaultShapeDef();
    _shape.density = 1.0f;
    AddShape(b2CreatePolygonShape(bodyId, &_shape, &_polygon));
    b2Body_ApplyMassFromShapes(bodyId);
}

void Krampus::PhysicsComponent::MakePolygon(const std::string& _shapeName)
{
    if (!b2Body_IsValid(bodyId)) return;
    DestroyShapes();
    shapeType = ShapeType::Polygon;
    polygonName = _shapeName.empty() ? std::string(ShapeAssets::DEFAULT_NAME) : _shapeName;

    // box2d takes convex polygons of at most B2_MAX_POLYGON_VERTICES points: a concave shape becomes several of them on the body.
    const Polygon2D& _polygon = ShapeAssets::Get(polygonName);
    debugSize = _polygon.Bounds().GetSize();
    builtScale = GetScale();
    b2ShapeDef _shape = b2DefaultShapeDef();
    _shape.density = 1.0f;
    for (std::vector<FVector2> _part : _polygon.ConvexParts(B2_MAX_POLYGON_VERTICES))
    {
        // Scaled like the sprite (a convex piece stays convex, even mirrored by a negative scale).
        for (FVector2& _point : _part) _point = FVector2(_point.x * builtScale.x, _point.y * builtScale.y);
        std::vector<b2Vec2> _points;
        for (const FVector2& _point : _part) _points.push_back(b2Vec2(_point.x / PX_TO_M, _point.y / PX_TO_M));
        const b2Hull _hull = b2ComputeHull(_points.data(), static_cast<int>(_points.size()));
        if (_hull.count < 3) continue;	// a sliver too thin for box2d
        const b2Polygon _box2dPolygon = b2MakePolygon(&_hull, 0.f);
        AddShape(b2CreatePolygonShape(bodyId, &_shape, &_box2dPolygon));
        polygonParts.push_back(_part);
    }
    if (shapeIds.empty())
    {
        LOG_WARNING("PhysicsComponent: the shape \"" + polygonName + "\" gave box2d nothing it can use (too small or flat?): a box is used");
        MakeRectangle(FVector2(std::max(1.f, static_cast<float>(debugSize.x)), std::max(1.f, static_cast<float>(debugSize.y))));
        return;
    }
    b2Body_ApplyMassFromShapes(bodyId);
}

void Krampus::PhysicsComponent::SetShapeBox(const FVector2& _size)
{
    matchSprite = false;
    MakeRectangle(_size);
}

void Krampus::PhysicsComponent::SetShapeCircle(const Float& _radius)
{
    matchSprite = false;
    MakeCircle(_radius);
}

void Krampus::PhysicsComponent::SetShapePolygon(const std::string& _shapeName)
{
    matchSprite = false;
    MakePolygon(_shapeName);
}

void Krampus::PhysicsComponent::SetShapeType(const ShapeType& _type)
{
    if (_type == shapeType) return;
    matchSprite = false;
    const FVector2 _oldSize = shapeType == ShapeType::Circle ? FVector2(debugRadius * 2.f, debugRadius * 2.f) : debugSize;
    if (_type == ShapeType::Rectangle) MakeRectangle(FVector2(std::max(1.f, _oldSize.x), std::max(1.f, _oldSize.y)));
    else if (_type == ShapeType::Circle) MakeCircle(std::max(1.f, std::min(_oldSize.x, _oldSize.y) * 0.5f));
    else MakePolygon(polygonName);
}


bool Krampus::PhysicsComponent::ContainsPoint(const FVector2& _worldPosition) const
{
    if (!b2Body_IsValid(bodyId)) return false;
    const b2Vec2 _point = b2Vec2(_worldPosition.x / PX_TO_M, _worldPosition.y / PX_TO_M);
    for (const b2ShapeId _shape : shapeIds)
        if (b2Shape_IsValid(_shape) && b2Shape_TestPoint(_shape, _point)) return true;
    return false;
}

Krampus::FVector2 Krampus::PhysicsComponent::GetWorldCenterOfMass() const
{
    if (!b2Body_IsValid(bodyId)) return GetPosition();
    const b2Vec2 _center = b2Body_GetWorldCenterOfMass(bodyId);
    return FVector2(_center.x, _center.y) * PX_TO_M;
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
	_category = static_cast<CollisionChannel>(filter.categoryBits);
	_mask = static_cast<CollisionChannel>(filter.maskBits);
	_groupIndex = filter.groupIndex;
}

void Krampus::PhysicsComponent::DrawDebug()
{
#ifdef DEBUG
	if (!b2Body_IsValid(bodyId)) return;
	SyncWithSprite();	// the editor does not tick the level: the outline follows the sprite there too

	// The shape where box2d has it (it is what collides), not where the actor is drawn.
	const b2Vec2 _position = b2Body_GetPosition(bodyId);
	const FVector2 _center = FVector2(_position.x, _position.y) * PX_TO_M;
	const Angle _rotation = Angle(b2Rot_GetAngle(b2Body_GetRotation(bodyId)));

	if (shapeType == ShapeType::Circle)
		Debug::DrawDebugCircle(GetLevel(), _center, debugRadius * std::max(std::abs(builtScale.x), std::abs(builtScale.y)), 20, Color::Cyan());
	else if (shapeType == ShapeType::Rectangle)
		Debug::DrawDebugRect(GetLevel(), _center, FVector2(debugSize.x * std::abs(builtScale.x), debugSize.y * std::abs(builtScale.y)), _rotation, Color::Cyan());
	else
		for (const std::vector<FVector2>& _part : polygonParts)	// the convex pieces box2d has
			Debug::DrawDebugPolygon(GetLevel(), Polygon2D::Transform(_part, _center, _rotation), Color::Cyan());

	// Center of mass: a yellow cross in a circle (it moves with SetMassData, and is off the middle for an uneven polygon).
	const FVector2 _mass = GetWorldCenterOfMass();
	Debug::DrawDebugCircle(GetLevel(), _mass, 6.f, 16, Color::Yellow());
	Debug::DrawFillRect(GetLevel(), _mass, FVector2(18.f, 1.5f), Angle(), Color::Yellow());
	Debug::DrawFillRect(GetLevel(), _mass, FVector2(1.5f, 18.f), Angle(), Color::Yellow());
#endif
}
