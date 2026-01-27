#pragma once
#include "Core/CoreMinimal.h"
#include "Utilities/System/Printable.h"

namespace Krampus
{

	class Transform : public IPrintable
	{
	public:
		FVector2 position;
		Angle rotation;
		FVector2 scale;

		inlin FVector2 Forward() const
		{
			return FVector2::Up().Rotated(rotation.ToRadians());
		}
		inlin FVector2 Back() const
		{
			return FVector2::Down().Rotated(rotation.ToRadians());
		}
		inlin FVector2 Right() const
		{
			return FVector2::Right().Rotated(rotation.ToRadians());
		}
		inlin FVector2 Left() const
		{
			return FVector2::Left().Rotated(rotation.ToRadians());
		}

		Transform();
		Transform(const FVector2& _position, const Angle& _rotation, const FVector2& _scale = FVector2(1.0f));

		void LookAt(const FVector2& _target);

		virtual std::string ToString() const override;
	};
}

