#pragma once
#include "Components/Component.h"
#include "Navigation/NavigationSystem.h"
#include "Reflection/EventList.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/Event.h"

// The pieces of the navigation, as components (add them in the Inspector, or spawn "NavMeshBoundsVolume" / "NavigationTestActor"):
//   NavMeshBoundsComponent   where a navigation mesh is built, for which agent size
//   NavModifierComponent     makes its actor block the navigation, or makes its area cost more (or less) to walk
//   NavLinkComponent         a jump, a ladder, a teleporter: two points that a path may join
//   NavAgentComponent        moves its actor along paths: MoveTo, follow, wander, with avoidance between agents
//   NavTestComponent         shows in the editor the path from its actor to a goal (to tune a level)

namespace Krampus
{
	/// <summary>The shape of an actor in world pixels: its PhysicsComponent's, else its SpriteComponent's. False if it has neither.</summary>
	bool NavShapeOf(Actor& _actor, NavSource& _source);

	/// <summary>
	/// A NavMesh Bounds Volume: the rectangle (centered on its actor) where a navigation mesh is built, for agents of agentRadius.
	/// Several volumes may share a place with different radii: each agent uses the one made for its size.
	/// </summary>
	class NavMeshBoundsComponent : public Component, public IDebuggable
	{
	public:
		FVector2	size		= FVector2(1600.f, 1200.f);	// pixels
		float		cellSize	= 8.f;		// precision: smaller cells follow obstacles closer, take longer to build
		float		agentRadius	= 16.f;		// the agents keep this far from obstacles
		int			tileSize	= 32;		// cells: the piece rebuilt when an obstacle moves
		bool		erodeEdges	= false;	// the border of the volume counts as a wall

		NavMeshBoundsComponent(Actor* _owner);

		void Construct() override;
		void Deconstruct() override;
		void Tick(const Float& _deltaTime) override;

		NavMeshSettings GetSettings() const;

		/// <summary>Builds it again now.</summary>
		void Rebuild();

		/// <returns>Its mesh (null until it is built).</returns>
		const NavMesh* GetMesh() const;

		std::string ToString() const override;

	private:
		void DrawDebug() override;
	};

	/// <summary>Area of a NavModifierComponent (saved as an int: never reorder).</summary>
	enum class NavArea
	{
		Blocked,	// an obstacle: no path goes through it
		Cost		// walkable, each pixel costs 'cost' (5: avoided when a detour is shorter than 5 times; 0.5: preferred)
	};

	/// <summary>
	/// Makes the shape of its actor (or a box / circle of its own) block the navigation or cost more to walk.
	/// Static physics bodies already block it: a modifier is for the rest (water, lava, a moving door, a road).
	/// </summary>
	class NavModifierComponent : public Component, public IDebuggable
	{
	public:
		int			area			= static_cast<int>(NavArea::Blocked);
		float		cost			= 5.f;
		bool		useOwnerShape	= true;		// the actor's physics shape, else its sprite; off: the shape below
		int			shape			= 0;		// 0 box, 1 circle (when useOwnerShape is off)
		FVector2	size			= FVector2(100.f, 100.f);
		float		radius			= 50.f;

		NavModifierComponent(Actor* _owner);

		/// <returns>True with its shape in world pixels.</returns>
		bool MakeSource(NavSource& _source) const;

	private:
		void DrawDebug() override;
	};

	/// <summary>
	/// An off-mesh link: a path may go from one end to the other (a jump over a gap, a ladder, a door, a teleporter).
	/// The ends are offsets from the actor; they snap onto the mesh.
	/// </summary>
	class NavLinkComponent : public Component, public IDebuggable
	{
	public:
		FVector2	startOffset		= FVector2(-64.f, 0.f);
		FVector2	endOffset		= FVector2(64.f, 0.f);
		bool		bidirectional	= true;
		float		cost			= 1.f;		// times its length: high = used only when nothing else works

		NavLinkComponent(Actor* _owner);

		NavLinkSource MakeSource() const;

	private:
		void DrawDebug() override;
	};

	/// <summary>What an agent is doing.</summary>
	enum class NavAgentStatus
	{
		Idle,
		Moving,
		Arrived,
		Failed		// no path (no mesh, or the goal and the agent are not on it)
	};

	/// <summary>
	/// Moves its actor along navigation paths, like Unity's NavMeshAgent or Unreal's MoveTo: MoveTo a point, follow an actor, wander.
	/// Accelerates, slows down on arrival, steers around other agents, goes through links, finds a new path when the goal or the
	/// mesh change. With a dynamic PhysicsComponent it moves the body through its velocity; else it moves the actor.
	/// </summary>
	class NavAgentComponent : public Component, public IDebuggable
	{
	public:
		float		speed				= 200.f;	// pixels per second
		float		acceleration		= 2000.f;	// pixels per second per second (0: at full speed at once)
		float		stoppingDistance	= 6.f;		// arrived when this close to the goal
		float		radius				= 16.f;		// its size: picks the mesh, and the room it takes for avoidance
		bool		rotateToMovement	= false;	// turns the actor toward where it goes
		float		rotationSpeed		= 540.f;	// degrees per second
		bool		avoidance			= true;		// steers around the other agents
		float		repathInterval		= 0.5f;		// seconds between two new paths while following a moving actor
		int			linkMode			= 0;		// how links are taken: 0 moving along them, 1 teleporting

		Event<>		onArrived;
		Event<>		onFailed;
		EventList	onArrivedCalls;		// set in the Inspector
		EventList	onFailedCalls;

		NavAgentComponent(Actor* _owner);

		void Construct() override;
		void Deconstruct() override;
		void Tick(const Float& _deltaTime) override;

		/// <summary>Goes to a place. Returns false when no path exists (onFailed is said too).</summary>
		bool MoveTo(const FVector2& _goal);

		/// <summary>Follows an actor (a new path when it moves) until Stop or until reaching it.</summary>
		bool MoveToActor(Actor* _target);

		/// <summary>Goes to a random place it can reach within _radius.</summary>
		bool Wander(const float& _radius);

		void Stop();

		NavAgentStatus GetStatus() const { return status; }
		bool IsMoving() const { return status == NavAgentStatus::Moving; }
		FVector2 GetVelocity() const { return velocity; }
		const NavPath& GetPath() const { return path; }
		FVector2 GetGoal() const { return goal; }
		float GetRemainingDistance() const;

	private:
		NavPath			path;
		int				next			= 1;		// index of the path point it goes to
		FVector2		goal;
		Actor*			target			= nullptr;
		FVector2		targetPathedAt;				// where the target was when the path was made
		float			sinceRepath		= 0.f;
		int				meshVersion		= -1;
		NavAgentStatus	status			= NavAgentStatus::Idle;
		FVector2		velocity;
		bool			onLink			= false;
		NavigationSystem*	system		= nullptr;

		bool MakePath();
		void Arrive();
		void Fail();
		FVector2 Avoid(const FVector2& _desired) const;
		void DrawDebug() override;
	};

	/// <summary>Shows the path from its actor to a goal while editing (Unreal's NavigationTestingActor), with how long it is.</summary>
	class NavTestComponent : public Component, public IDebuggable
	{
	public:
		FVector2	goalOffset		= FVector2(400.f, 0.f);	// the goal, from the actor
		float		agentRadius		= 16.f;

		NavTestComponent(Actor* _owner);

		/// <returns>The path as it is now.</returns>
		NavPath Compute() const;

	private:
		void DrawDebug() override;
	};
}
