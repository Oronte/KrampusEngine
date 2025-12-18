#pragma once
#include "Rectangle.h"
#include "Angle.h"

namespace Krampus
{
	struct CollisionInfo
	{
		FVector2 normal;
		float penetration = 0.f;
		FVector2 contactPoint;
		class CollisionComponent* collision;
		bool hit = false;
	};

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
		static RaycastHitInfo Raycast(const FVector2& _rayOrigin, const FVector2& _rayDir, const FRect& _rect, const float _rectRot);

	private:
		static void GetAxes(const Angle& _rotation, FVector2 _axes[2]);
		static float ProjectOBB(const FVector2& _size, const Angle& _rotation, const FVector2& _axis);
	};

}

