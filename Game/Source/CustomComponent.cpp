#include "CustomComponent.h"

CustomComponent::CustomComponent(Krampus::Actor* _owner)
	: Component(_owner)
{
	timer = CreateTimer([this]() 
		{ 
			transform.position += Krampus::FVector2(50.0f, 35.0f);
			transform.rotation += Krampus::Angle(Krampus::FMath::pi / 2.0f);
		}, 3.5f, true, false);
}

void CustomComponent::Construct()
{
	Super::Construct();

	eventHandle = onComponentDestroyed.AddListener(this, &CustomComponent::PrintTimer);
}

void CustomComponent::BeginPlay()
{
	Super::BeginPlay();

	timer->Play();
}

void CustomComponent::Tick(const float& _deltaTime)
{
	Super::Tick(_deltaTime);
}

void CustomComponent::Deconstruct()
{
	Super::Deconstruct();
}

void CustomComponent::BeginDestroy()
{
	Super::BeginDestroy();

	onComponentDestroyed.Broadcast(timer);
}

void CustomComponent::PrintTimer(Krampus::Timer* _timer)
{
	Krampus::Logger::PrintLog(Krampus::VerbosityType::Display, _timer, "(Debug data)");
	LOG_MSG(_timer);
	LOG_WARNING(_timer);
	LOG_ERROR(_timer);
	//LOG_FATAL(_timer);
}
