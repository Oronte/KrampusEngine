#pragma once
#include "Component.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/CollisionChannel.h"
#include "Graphics/Shapes/ShapeObject.h"
#include "box2d/box2d.h"

namespace Krampus
{
	/// <summary>
	/// Component that integrates Box2D 2D rigid body physics simulation with an actor.
	/// </summary>
    class PhysicsComponent : public Component, public IDebuggable
    {
        b2BodyId    bodyId;
		b2ShapeId   shapeId;

        Event<>::ListenerHandle collisionResolveHandle;
        Event<FVector2>::ListenerHandle moveHandle;
        Event<Angle>::ListenerHandle rotateHandle;
        Event<FVector2>::ListenerHandle scaleHandle;

    public:
		/// <summary>
		/// Constructs a PhysicsComponent that always has the shape of the owner's SpriteComponent (see SetMatchSprite).
		/// Without a sprite it is a box of 100 x 100 until one is added.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
        PhysicsComponent(Actor* _owner);

		/// <summary>
		/// Constructs a PhysicsComponent with a circular Box2D body of the specified radius (a custom shape: it does not follow the sprite).
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_radius">Radius of the physics circle.</param>
        PhysicsComponent(Actor* _owner, const Float& _radius);

		/// <summary>
		/// Constructs a PhysicsComponent with a box Box2D body of the specified dimensions (a custom shape: it does not follow the sprite).
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		/// <param name="_size">Width and height of the physics box.</param>
        PhysicsComponent(Actor* _owner, const FVector2& _size);

		virtual void Construct() override;

		/// <summary>
		/// Follows the sprite: its shape is changed when the sprite's one is (checked every frame).
		/// </summary>
		virtual void Tick(const Float& _deltaTime) override;

		/// <summary>
		/// Lifecycle callback invoked when the component is deconstructed, destroying the Box2D body.
		/// </summary>
		virtual void Deconstruct() override;

		/// <summary>
		/// Callback invoked after collision resolution to synchronize transforms.
		/// </summary>
        void OnCollisionResolve();

		/// <summary>
		/// Body type as box2d numbers it: 0 static, 1 kinematic, 2 dynamic.
		/// </summary>
		int GetBodyType() const;

		/// <summary>
		/// Sets the body type (0 static, 1 kinematic, 2 dynamic).
		/// </summary>
		void SetBodyType(const int& _type);

		/// <summary>
		/// Configures the Box2D body type as dynamic (fully simulated by forces and gravity).
		/// </summary>
        void SetDynamic();

		/// <summary>
		/// Configures the Box2D body type as static (immovable, unaffected by forces).
		/// </summary>
        void SetStatic();

		/// <summary>
		/// Configures the Box2D body type as kinematic (moved only by script/velocity, unaffected by forces).
		/// </summary>
        void SetKinematic();

		/// <summary>
		/// Sets the linear velocity of an object.
		/// </summary>
		/// <param name="_velocity">The new linear velocity to set, represented as an FVector2.</param>
		void SetLinearVelocity(const FVector2& _velocity);

		/// <summary>
		/// Retrieves the current linear velocity as a 2D vector.
		/// </summary>
		/// <returns>A FVector2 representing the current linear velocity.</returns>
		FVector2 GetLinearVelocity() const;

		/// <summary>
		/// Sets the angular velocity to the specified value.
		/// </summary>
		/// <param name="_velocity">The new angular velocity to set.</param>
		void SetAngularVelocity(const Float& _velocity);

		/// <summary>
		/// Retrieves the current angular velocity.
		/// </summary>
		/// <returns>The current angular velocity as a Float.</returns>
		Float GetAngularVelocity() const;

		/// <summary>
		/// Sets the linear damping value.
		/// </summary>
		/// <param name="_damping">The linear damping value to set.</param>
		void SetLinearDamping(const Float& _damping);

		/// <summary>
		/// Retrieves the linear damping value.
		/// </summary>
		/// <returns>The linear damping value as a Float.</returns>
		Float GetLinearDamping() const;

		/// <summary>
		/// Sets the angular damping value.
		/// </summary>
		/// <param name="_damping">The new angular damping value to set.</param>
		void SetAngularDamping(const Float& _damping);

		/// <summary>
		/// Retrieves the angular damping value.
		/// </summary>
		/// <returns>The current angular damping as a Float.</returns>
		Float GetAngularDamping() const;

		/// <summary>
		/// Sets the gravity scale factor.
		/// </summary>
		/// <param name="_scale">The new gravity scale to apply.</param>
		void SetGravityScale(const Float& _scale);

		/// <summary>
		/// Retrieves the current gravity scale value.
		/// </summary>
		/// <returns>The gravity scale as a Float.</returns>
		Float GetGravityScale() const;

		/// <summary>
		/// Sets the mass data, including mass, inertia, and center of mass for an object.
		/// </summary>
		/// <param name="_mass">The mass value to set.</param>
		/// <param name="_inertia">The moment of inertia to set.</param>
		/// <param name="_center">The center of mass as a 2D vector.</param>
		void SetMassData(const Float& _mass, const Float& _inertia, const FVector2& _center);
		
		/// <summary>
		/// Retrieves the mass, inertia, and center of mass data.
		/// </summary>
		/// <param name="_mass">Reference to a variable that will receive the mass value.</param>
		/// <param name="_inertia">Reference to a variable that will receive the inertia value.</param>
		/// <param name="_center">Reference to a variable that will receive the center of mass as a 2D vector.</param>
		void GetMassData(Float& _mass, Float& _inertia, FVector2& _center) const;
		
		/// <summary>
		/// Calculates and applies the mass of an object based on its constituent shapes.
		/// </summary>
		void ApplyMassFromShapes();

		/// <summary>
		/// Sets whether the rotation is fixed.
		/// </summary>
		/// <param name="_fixed">A boolean value indicating if the rotation should be fixed (true) or not (false).</param>
		void SetFixedRotation(const bool& _fixed);

		/// <summary>
		/// Enables or disables sleeping for the physics object.
		/// </summary>
		/// <param name="_enable">A boolean value indicating if sleeping should be enabled (true) or disabled (false).</param>
		void EnableSleeping(const bool& _enable);
		
		/// <summary>
		/// Checks if sleeping is allowed for the physics object.
		/// </summary>
		/// <returns>A boolean value indicating if sleeping is allowed (true) or not (false).</returns>
		bool IsSleepingAllowed() const;
		
		/// <summary>
		/// Sets the sleep threshold for the physics object.
		/// </summary>
		/// <param name="_threshold">The sleep threshold value.</param>
		void SetSleepThreshold(const Float& _threshold);
		
		/// <summary>
		/// Retrieves the sleep threshold for the physics object.
		/// </summary>
		/// <returns>The sleep threshold value.</returns>
		Float GetSleepThreshold() const;
		
		/// <summary>
		/// Sets the awake state for the physics object.
		/// </summary>
		/// <param name="_awake">A boolean value indicating if the object should be awake (true) or asleep (false).</param>
		void SetAwake(const bool& _awake);
		
		/// <summary>
		/// Checks if the object is currently awake.
		/// </summary>
		/// <returns>true if the object is awake; otherwise, false.</returns>
		bool IsAwake() const;
		
		/// <summary>
		/// Enables the physics component.
		/// </summary>
		void Enable();
		
		/// <summary>
		/// Disables the physics component.
		/// </summary>
		void Disable();
		
		/// <summary>
		/// Enables or disables contact events for the physics object.
		/// </summary>
		/// <param name="_enable">A boolean value indicating if contact events should be enabled (true) or disabled (false).</param>
		void EnableContactEvents(bool _enable);
		
		/// <summary>
		/// Sets the bullet state for the physics object.
		/// </summary>
		/// <param name="_bullet">A boolean value indicating if the object should be treated as a bullet (true) or not (false).</param>
		void SetBullet(const bool& _bullet);

		/// <summary>
		/// Sets the collision filter for the physics object, defining its category, mask, and group index.
		/// </summary>
		/// <param name="_category"></param>
		/// <param name="_mask"></param>
		/// <param name="groupIndex"></param>
		void SetFilter(const CollisionChannel& _category, const CollisionChannel& _mask, int32_t groupIndex = 0);

		/// <summary>Whether the rotation is fixed.</summary>
		bool IsFixedRotation() const;

		/// <summary>Whether the body takes part in the simulation (see Enable / Disable).</summary>
		bool IsEnabled() const;

		/// <summary>Whether the shape reports contact events.</summary>
		bool AreContactEventsEnabled() const;

		/// <summary>Whether the body is treated as a bullet (continuous collision against other dynamic bodies).</summary>
		bool IsBullet() const;

		/// <summary>Gets the collision filter set by SetFilter.</summary>
		void GetFilter(CollisionChannel& _category, CollisionChannel& _mask, int32_t& _groupIndex) const;

		// ---- Shape of the body (the body keeps its velocity and settings when its shape changes)
		// Sizes are given without the scale of the actor: the body is always scaled like the actor (and its sprite).

		/// <summary>
		/// True (the default): the shape is always the one of the owner's SpriteComponent (type, size, radius, polygon).
		/// False: a custom shape, set with the functions below. Setting a shape turns it off.
		/// </summary>
		void SetMatchSprite(const bool& _match);
		bool IsMatchingSprite() const { return matchSprite; }

		/// <summary>Takes the shape of the sprite at once if it changed (done every frame by Tick when the shape matches the sprite).</summary>
		void SyncWithSprite();

		/// <summary>A box of this size, centered on the actor (a custom shape).</summary>
		void SetShapeBox(const FVector2& _size);

		/// <summary>A circle of this radius, centered on the actor (a custom shape).</summary>
		void SetShapeCircle(const Float& _radius);

		/// <summary>
		/// The polygon of a shape of Content/Shapes (concave included: box2d gets it as several convex pieces of at most 8 points).
		/// A custom shape.
		/// </summary>
		void SetShapePolygon(const std::string& _shapeName);

		/// <summary>Turns the shape into a box, a circle or a polygon of about the same size (a custom shape).</summary>
		void SetShapeType(const ShapeType& _type);

		ShapeType GetShapeType() const { return shapeType; }
		FVector2 GetBoxSize() const { return debugSize; }
		Float GetCircleRadius() const { return debugRadius; }
		const std::string& GetPolygonName() const { return polygonName; }

		/// <summary>Whether a world position (pixels) is inside the box2d shape of the body (every piece of a concave polygon).</summary>
		bool ContainsPoint(const FVector2& _worldPosition) const;

		/// <summary>Center of mass of the body, in world pixels (drawn as a yellow cross by the debug outline).</summary>
		FVector2 GetWorldCenterOfMass() const;

    private:
        void DrawDebug() override;	// outline of the box2d shape, where the body is

        ShapeType   shapeType   = ShapeType::Rectangle;	// what the box2d shapes are, to outline them and rebuild them
        Float       debugRadius = 0.f;						// without the scale
        FVector2    debugSize;								// without the scale
        std::string polygonName = ShapeAssets::DEFAULT_NAME;
        bool        matchSprite = true;
        FVector2    builtScale  = FVector2(1.f, 1.f);		// scale of the actor when the box2d shapes were made
        std::vector<std::vector<FVector2>>	polygonParts;	// the convex pieces given to box2d (pixels, around the actor, scaled)
        std::vector<b2ShapeId>				shapeIds;		// every box2d shape of the body (shapeId is the first)
        b2Filter    filter      = b2DefaultFilter();		// kept: a new shape gets it too
        ListenerHandle shapeSavedHandle;

        void Init();
        void MakeCircle(const Float& _radius);
        void MakeRectangle(const FVector2& _size);
        void MakePolygon(const std::string& _shapeName);
        void Rebuild();	// the same shape again, at the scale of the actor
        void DestroyShapes();
        void AddShape(b2ShapeId _shape);
    };
}
