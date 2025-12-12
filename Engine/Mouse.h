#pragma once
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
		INLINE void SetPosition(const IVector2& _position)
		{
			sf::Mouse::setPosition(_position);
		}
		INLINE bool HasMoved() const
		{
			return position == lastPosition;
		}
		INLINE void Update()
		{
			lastPosition = position;
			position = sf::Mouse::getPosition();
		}
	};

}