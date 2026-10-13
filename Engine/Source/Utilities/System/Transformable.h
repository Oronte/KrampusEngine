#pragma once
#include "Utilities/Math/Transform.h"

namespace Krampus
{

	class ITransformable
	{
	public:
		virtual void SetPosition(const FVector2& _newPosition) = 0;
		NO_DISCARD virtual FVector2 GetPosition() const = 0;
		virtual void Move(const FVector2& _offset)
		{
			SetPosition(GetPosition() + _offset);
		}

		virtual void SetRotation(const Angle& _newRotation) = 0;
		NO_DISCARD virtual Angle GetRotation() const = 0;
		virtual void Rotate(const Angle& _offset)
		{
			SetRotation(GetRotation() + _offset);
		}

		virtual void SetScale(const FVector2& _newScale) = 0;
		NO_DISCARD virtual FVector2 GetScale() const = 0;
		virtual void Scale(const FVector2& _offset)
		{
			SetScale(GetScale() + _offset);
		}

		virtual void SetTransform(const Transform& _transform)
		{
			SetPosition(_transform.position);
			SetRotation(_transform.rotation);
			SetScale(_transform.scale);
		}


		NO_DISCARD virtual inline FVector2 GetUpVector() const
		{
			return FVector2::Up().Rotated(GetRotation().ToRadians());
		}
		NO_DISCARD virtual inline FVector2 GetDownVector() const
		{
			return FVector2::Down().Rotated(GetRotation().ToRadians());
		}
		NO_DISCARD virtual inline FVector2 GetRightVector() const
		{
			return FVector2::Right().Rotated(GetRotation().ToRadians());
		}
		NO_DISCARD virtual inline FVector2 GetLeftVector() const
		{
			return FVector2::Left().Rotated(GetRotation().ToRadians());
		}

		NO_DISCARD virtual inline Angle GetAngleTo(const FVector2& _target) const
		{
			return Angle((_target - GetPosition()).AtanToRadian());
		}
		virtual inline void LookAt(const FVector2& _target)
		{
			SetRotation(GetAngleTo(_target));
		}
	};

}