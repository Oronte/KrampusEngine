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
		FVector2 origin;

		Transform();
		Transform(const FVector2& _position, const Angle& _rotation, const FVector2& _scale = FVector2(1.0f));
		Transform(const FVector2& _origin, const FVector2& _position,
			const Angle& _rotation, const FVector2& _scale = FVector2(1.0f));


		virtual std::string ToString() const override;
	};
}

