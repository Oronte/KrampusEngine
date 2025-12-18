#pragma once
#include "Core/CoreMinimal.h"
#include "Utilities/System/Printable.h"

namespace Krampus
{

	class Transform : public IPrintable
	{
	public:
		FVector2 origin;
		FVector2 position;
		Angle rotation;
		FVector2 scale;

		INLINE FVector2 Up() const
		{
			return FVector2::Up().Rotated(rotation.ToRadians());
		}
		INLINE FVector2 Down() const
		{
			return FVector2::Down().Rotated(rotation.ToRadians());
		}
		INLINE FVector2 Right() const
		{
			return FVector2::Right().Rotated(rotation.ToRadians());
		}
		INLINE FVector2 Left() const
		{
			return FVector2::Left().Rotated(rotation.ToRadians());
		}

		Transform();
		Transform(const FVector2& _position, const Angle& _rotation, const FVector2& _scale = FVector2(1.0f));
		Transform(const FVector2& _origin, const FVector2& _position,
			const Angle& _rotation, const FVector2& _scale = FVector2(1.0f));


		virtual std::string ToString() const override;
	};
}

