#include "CameraActorExemple.h"
#include "Core/Engine.h"
#include "Managers/InputManager.h"
#include "Components/SpriteComponent.h"
#include "Graphics/Mouse.h"

Krampus::Exemple::CameraActorExemple::CameraActorExemple(Level* _level)
	: Actor(_level)
{
	camera = CreateComponent<CameraComponent>();
	camera->attachedToOwner = true;
	RectangleShapeData _data;
	_data.size = FVector2(685.0f, 414.0f) / 3.0f;
	_data.texturePath = "Camera";
	CreateComponent<SpriteComponent>(_data)->SetZOrder(ZOrder::Debug);
}

void Krampus::Exemple::CameraActorExemple::Construct()
{
	Super::Construct();

	onWheelScroll = GetWorld()->GetInputManager()->MouseWheelScroll.AddListener([this](Float _factor)
		{
			Rotate(Angle(_factor * 3.0f, true));
		});
}

void Krampus::Exemple::CameraActorExemple::BeginPlay()
{
	Super::BeginPlay();

	camera->SetCurrent();
	//GetWorld()->GetMouse()->Hide();
}

void Krampus::Exemple::CameraActorExemple::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);

	SetActorPosition(GetWorld()->GetMouse()->GetScreenPosition());
}

void Krampus::Exemple::CameraActorExemple::Deconstruct()
{
	Super::Deconstruct();

	//GetWorld()->GetMouse()->Show();
}
