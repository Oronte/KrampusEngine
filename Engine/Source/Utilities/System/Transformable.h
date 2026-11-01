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

		/// <summary>
		/// The transform relative to the parent (an actor or a widget in a hierarchy); the world one without a parent.
		/// </summary>
		NO_DISCARD virtual Transform GetLocalTransform() const
		{
			return Transform(GetPosition(), GetRotation(), GetScale());
		}

		/// <summary>
		/// Sets the transform relative to the parent; the world one without a parent.
		/// </summary>
		virtual void SetLocalTransform(const Transform& _local)
		{
			SetTransform(_local);
		}

		/// <summary>
		/// True when the transform is relative to a parent (GetLocalTransform differs from the world one).
		/// </summary>
		NO_DISCARD virtual bool HasParentTransform() const
		{
			return false;
		}

		/// <summary>
		/// The world transform of a child: its local one placed in its parent (scaled along the parent's axes, then turned and moved).
		/// </summary>
		static Transform ComposeTransform(const Transform& _parent, const Transform& _local)
		{
			const float _turn = _parent.rotation.ToRadians();
			const FVector2 _offset(_local.position.x * _parent.scale.x, _local.position.y * _parent.scale.y);
			return Transform(_parent.position + _offset.Rotated(_turn), _parent.rotation + _local.rotation,
				FVector2(_parent.scale.x * _local.scale.x, _parent.scale.y * _local.scale.y));
		}

		/// <summary>
		/// The local transform that gives _world in _parent (the opposite of ComposeTransform). A parent axis at 0 has no size to
		/// divide by: the child keeps its world value on that axis.
		/// </summary>
		static Transform RelativeTransform(const Transform& _parent, const Transform& _world)
		{
			const auto _divide = [](const float _value, const float _by) { return _by != 0.f ? _value / _by : _value; };
			const FVector2 _offset = (_world.position - _parent.position).Rotated(-static_cast<float>(_parent.rotation.ToRadians()));
			return Transform(FVector2(_divide(_offset.x, _parent.scale.x), _divide(_offset.y, _parent.scale.y)), _world.rotation - _parent.rotation,
				FVector2(_divide(_world.scale.x, _parent.scale.x), _divide(_world.scale.y, _parent.scale.y)));
		}
	};

}