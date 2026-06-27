#include "ActorExemple.h"

Krampus::Exemple::ActorExemple::ActorExemple(Krampus::Level* _level)
	: Actor(_level)
{
	// Used to create component 
	
	component = CreateComponent<ComponentExemple>();
}

Krampus::Exemple::ActorExemple::ActorExemple(Krampus::Level* _level, Float _exempleParameter)
	: Actor(_level)
{
	component = CreateComponent<ComponentExemple>(_exempleParameter);
}

void Krampus::Exemple::ActorExemple::Construct()
{
	Super::Construct();

	// Used to Initialize Components and variables
}

void Krampus::Exemple::ActorExemple::BeginPlay()
{
	Super::BeginPlay();

	// Used to Start an action directly 
}

void Krampus::Exemple::ActorExemple::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);

	// Used to Execute something every frames
}

void Krampus::Exemple::ActorExemple::Deconstruct()
{
	Super::Deconstruct();

	// Used to Uninitialize Components and variables
}

void Krampus::Exemple::ActorExemple::BeginDestroy()
{
	Super::BeginDestroy();

	// Used to Execute something before Destroyed
}
