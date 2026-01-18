#pragma once
#include "Rectangle.h"
#include "Angle.h"

namespace Krampus
{

	struct CollisionInfo
	{
		FVector2 normal;
		float penetration = 0.f;
		FVector2 avrageContactPoint;
		std::vector<FVector2> contacts;
		class CollisionComponent* collision = nullptr;
		bool hit = false;
	};

	// TODO raycast
	struct RaycastHitInfo
	{
		FVector2 normal;
		float distance = 0.f;
		FVector2 point;
		bool hit = false;
	};

	class Physics
	{
	public:
		static bool CircleToCircle(const FVector2& _aPos, const float& _aRadius,
			const FVector2& _bPos, const float& _bRadius,
			CollisionInfo& _aInfo, CollisionInfo& _bInfo);
		static bool RectToRectOBB(const FRect& _aRect, const Angle& _aRot,
			const FRect& _bRect, const Angle& _bRot,
			CollisionInfo& _aInfo, CollisionInfo& _bInfo);
		static bool RectToRectAABB(const FRect& _aRect, const FRect& _bRect,
			CollisionInfo& _aInfo, CollisionInfo& _bInfo);
		static bool CircleToRect(const FVector2& _circlePos, const float& _radius,
			const FRect& _rect, const Angle& _rectRot,
			CollisionInfo& _circleInfo, CollisionInfo& _rectInfo);

		static bool Contains(const FVector2& _point, const FVector2& _circlePos,
			const float& _radius, CollisionInfo& _info);
		static bool Contains(const FVector2& _point, const FVector2& _pos,
			const FVector2& _size, CollisionInfo& _info);
		static bool Contains(const FVector2& _point, const FVector2& _pos,
			const FVector2& _size, const Angle& _rot, CollisionInfo& _info);

	private:
		static void GetAxes(const Angle& _rotation, FVector2 _axes[2]);
		static float ProjectOBB(const FVector2& _size, const Angle& _rotation, const FVector2& _axis);
        static std::array<FVector2, 4> GetCorners(const FRect& _rect, const Angle& _rot);
        static bool SegmentIntersect(const FVector2& _p, const FVector2& _r,
            const FVector2& _q, const FVector2& _s, FVector2& _outPoint);
		static std::vector<FVector2> CalculateOBBContactPoints(const FRect& _aRect, const Angle& _aRot,
			const FRect& _bRect, const Angle& _bRot);
		static FVector2 ComputeAverageContactPoint(const std::vector<FVector2>& _contacts);
		static std::vector<FVector2> CircleToRectContacts(const FVector2& _circlePos, 
			float _radius, const FRect& _rect, float _rectRot);
		static std::vector<FVector2> CircleCircleIntersections(const FVector2& _c1, float _r1,
			const FVector2& _c2, float _r2);
	};

}

