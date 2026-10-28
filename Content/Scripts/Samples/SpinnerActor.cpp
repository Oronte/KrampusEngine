#include "SpinnerActor.h"

namespace Krampus
{

	SpinnerActor::SpinnerActor(Level* _level)
		: Actor(_level)
	{
		name = NAME_OF(SpinnerActor);
	}

	void SpinnerActor::Tick(const Float& _deltaTime)
	{
		Actor::Tick(_deltaTime);

		const float _step = (clockwise ? 1.f : -1.f) * static_cast<float>(speed) * static_cast<float>(_deltaTime);
		const float _before = GetRotation().ToDegrees();
		SetRotation(Angle::FromDegrees(_before + _step));

		if (_before + _step >= 360.f * (static_cast<int>(turns) + 1))
			turns = Int(static_cast<int>(turns) + 1);
	}

}
