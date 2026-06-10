#pragma once
#include "Utilities/Math/Transform.h"

namespace Krampus
{

	class ITransformable
	{
	public:
		virtual void SetActorPosition(const FVector2& _newPosition) = 0;
		virtual FVector2 GetActorPosition() const = 0;
		virtual void Move(const FVector2& _offset)
		{
			SetActorPosition(GetActorPosition() + _offset);
		}

		virtual void SetActorRotation(const Angle& _newRotation) = 0;
		virtual Angle GetActorRotation() const = 0;
		virtual void Rotate(const Angle& _offset)
		{
			SetActorRotation(GetActorRotation() + _offset);
		}

		virtual void SetActorScale(const FVector2& _newScale) = 0;
		virtual FVector2 GetActorScale() const = 0;
		virtual void Scale(const FVector2& _offset)
		{
			SetActorScale(GetActorScale() + _offset);
		}

		virtual void SetActorTransform(const Transform& _transform)
		{
			SetActorPosition(_transform.position);
			SetActorRotation(_transform.rotation);
			SetActorScale(_transform.scale);
		}


		virtual inline FVector2 GetActorUpVector() const
		{
			return FVector2::Up().Rotated(GetActorRotation().ToRadians());
		}
		virtual inline FVector2 GetActorDownVector() const
		{
			return FVector2::Down().Rotated(GetActorRotation().ToRadians());
		}
		virtual inline FVector2 GetActorRightVector() const
		{
			return FVector2::Right().Rotated(GetActorRotation().ToRadians());
		}
		virtual inline FVector2 GetActorLeftVector() const
		{
			return FVector2::Left().Rotated(GetActorRotation().ToRadians());
		}

		virtual inline Angle GetAngleTo(const FVector2& _target) const
		{
			return Angle((_target - GetActorPosition()).AtanToRadian());
		}
		virtual inline void LookAt(const FVector2& _target)
		{
			SetActorRotation(GetAngleTo(_target));
		}
	};

}