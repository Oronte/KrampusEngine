#include "CollisionChannel.h"
#include "box2d/box2d.h"

::b2Filter Krampus::MakeFilter(CollisionChannel _category, CollisionChannel _mask, int32_t _groupIndex) noexcept
{
	b2Filter _filter = b2DefaultFilter();
	_filter.categoryBits = ToBits(_category);
	_filter.maskBits = ToBits(_mask);
	_filter.groupIndex = _groupIndex;
	return _filter;
}