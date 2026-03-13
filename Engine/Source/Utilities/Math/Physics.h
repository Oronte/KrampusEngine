#pragma once
#include "Rectangle.h"
#include "Angle.h"

namespace Krampus
{

	struct CollisionInfo
	{
		FVector2					normal				= FVector2::Zero();
		Float						penetration			= 0.0f;
		FVector2					avrageContactPoint	= FVector2::Zero();
		std::vector<FVector2>		contacts;
		class CollisionComponent*	collision			= nullptr;
		Bool						hit					= false;
	};

	// TODO raycast
	struct RaycastHitInfo
	{
		FVector2	normal		= FVector2::Zero();
		Float		distance	= 0.f;
		FVector2	point		= FVector2::Zero();
		Bool		hit			= false;
	};

	class Physics
	{
	public:
		static Bool CircleToCircle(const FVector2& _aPos, const Float& _aRadius,
			const FVector2& _bPos, const Float& _bRadius,
			CollisionInfo& _aInfo, CollisionInfo& _bInfo);
		static Bool RectToRectOBB(const FRect& _aRect, const Angle& _aRot,
			const FRect& _bRect, const Angle& _bRot,
			CollisionInfo& _aInfo, CollisionInfo& _bInfo);
		static Bool RectToRectAABB(const FRect& _aRect, const FRect& _bRect,
			CollisionInfo& _aInfo, CollisionInfo& _bInfo);
		static Bool CircleToRect(const FVector2& _circlePos, const Float& _radius,
			const FRect& _rect, const Angle& _rectRot,
			CollisionInfo& _circleInfo, CollisionInfo& _rectInfo);

		static Bool Contains(const FVector2& _point, const FVector2& _circlePos,
			const Float& _radius, CollisionInfo& _info);
		static Bool Contains(const FVector2& _point, const FVector2& _pos,
			const FVector2& _size, CollisionInfo& _info);
		static Bool Contains(const FVector2& _point, const FVector2& _pos,
			const FVector2& _size, const Angle& _rot, CollisionInfo& _info);

	private:
		static void GetAxes(const Angle& _rotation, FVector2 _axes[2]);
		static Float ProjectOBB(const FVector2& _size, const Angle& _rotation, const FVector2& _axis);
        static std::array<FVector2, 4> GetCorners(const FRect& _rect, const Angle& _rot);
        static Bool SegmentIntersect(const FVector2& _p, const FVector2& _r,
            const FVector2& _q, const FVector2& _s, FVector2& _outPoint);
		static std::vector<FVector2> CalculateOBBContactPoints(const FRect& _aRect, const Angle& _aRot,
			const FRect& _bRect, const Angle& _bRot);
		static FVector2 ComputeAverageContactPoint(const std::vector<FVector2>& _contacts);
		static std::vector<FVector2> CircleToRectContacts(const FVector2& _circlePos, 
			Float _radius, const FRect& _rect, Float _rectRot);
		static std::vector<FVector2> CircleCircleIntersections(const FVector2& _c1, Float _r1,
			const FVector2& _c2, Float _r2);
	};

}

