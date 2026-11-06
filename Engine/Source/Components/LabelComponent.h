#pragma once
#include "BasicLabel.h"

namespace Krampus
{

	/// <summary>A text drawn in the world, at its actor's transform.</summary>
	class LabelComponent : public BasicLabel<Component>
	{
	public:
		LabelComponent(Actor* _owner, const TextData& _data) : BasicLabel(_owner, _data) { name = NAME_OF(LabelComponent); }
	};

}
