#pragma once
#include "Utilities/Math/Vector2D.h"
#include "Utilities/Singleton.h"

namespace Krampus
{

	class Mouse : public Singleton<Mouse>
	{
		IVector2 position;
		IVector2 lastPosition;

	public:
		INLINE IVector2 GetPosition() const noexcept
		{
			return position;
		}
		INLINE void SetPosition(const IVector2& _position) noexcept
		{
			sf::Mouse::setPosition(_position);
			Update();
		}
		INLINE bool HasMoved() const noexcept
		{
			return position != lastPosition;
		}

		INLINE void Update()
		{
			lastPosition = position;
			position = sf::Mouse::getPosition();
		}
	};


}
