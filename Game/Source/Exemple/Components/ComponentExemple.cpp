#include "ComponentExemple.h"

Krampus::Exemple::ComponentExemple::ComponentExemple(Krampus::Actor* _owner)
	: Component(_owner)
{
}

Krampus::Exemple::ComponentExemple::ComponentExemple(Krampus::Actor* _owner, Float _exempleParameter)
	: Component(_owner)
{
}

void Krampus::Exemple::ComponentExemple::Construct()
{
	Super::Construct();

	// Used to Initialize variables
}

void Krampus::Exemple::ComponentExemple::BeginPlay()
{
	Super::BeginPlay();

	// Used to Start an action directly 
}

void Krampus::Exemple::ComponentExemple::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);

	// Used to Execute something every frames
}

void Krampus::Exemple::ComponentExemple::Deconstruct()
{
	Super::Deconstruct();

	// Used to Uninitialize variables
}

void Krampus::Exemple::ComponentExemple::BeginDestroy()
{
	Super::BeginDestroy();

	// Used to Execute something before Destroyed
}