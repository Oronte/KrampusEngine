#include "Test.h"

namespace Krampus
{

Test::Test(Level* _level)
	: Actor(_level)
{
	name = NAME_OF(Test);
}

void Test::BeginPlay()
{
	Actor::BeginPlay();
}

void Test::Tick(const Float& _deltaTime)
{
	Actor::Tick(_deltaTime);
}

}
