#include "BasicImage.h"
#include "Actors/Actor.h"
#include "UI/Widgets/Widget.h"
#include "Graphics/Window.h"
#include "GameFramework/Level.h"
#include "Utilities/Math/Polygon2D.h"

template <typename Base>
Krampus::BasicImage<Base>::BasicImage(typename Base::OwnerType* _owner, const CircleShapeData& _data)
	: DrawnComponent<Base>(_owner), shape(std::make_unique<ShapeObject>(_data))
{
	FollowOwner(_owner);
}

template <typename Base>
Krampus::BasicImage<Base>::BasicImage(typename Base::OwnerType* _owner, const RectangleShapeData& _data)
	: DrawnComponent<Base>(_owner), shape(std::make_unique<ShapeObject>(_data))
{
	FollowOwner(_owner);
}

template <typename Base>
void Krampus::BasicImage<Base>::FollowOwner(typename Base::OwnerType* _owner)
{
	this->SetZOrder(ZOrder::Actors);
	onOwnerMoveHandle = _owner->onMove.AddListener([this](FVector2 _position) { shape->GetShape()->SetPosition(_position); });
	onOwnerRotateHandle = _owner->onRotate.AddListener([this](Angle _rotation) { shape->GetShape()->SetRotation(_rotation); });
	onOwnerScaleHandle = _owner->onScale.AddListener([this](FVector2 _scale) { shape->GetShape()->SetScale(_scale); });
}

template <typename Base>
void Krampus::BasicImage<Base>::Construct()
{
	DrawnComponent<Base>::Construct();
	shape->GetShape()->SetPosition(this->GetPosition());
	shape->GetShape()->SetRotation(this->GetRotation());
	shape->GetShape()->SetScale(this->GetScale());
}

template <typename Base>
void Krampus::BasicImage<Base>::Draw(Window& _window)
{
	if (!this->isActive) return;
	sf::RenderStates _states;
	const sf::Shape& _drawn = *shape->GetShape()->Get();
	if (material.Apply(_states, _drawn.getTexture(), sf::Vector2f(_window.GetRenderWindow().getSize()))) _window.GetRenderWindow().draw(_drawn, _states);
	else _window.Draw(*shape);
}

template <typename Base>
void Krampus::BasicImage<Base>::DrawDebug()
{
#ifdef DEBUG
	if (!IsDebugging()) return;

	const FVector2 _position = this->GetPosition();
	const FVector2 _scale = this->GetScale();
	switch (shape->GetShapeType())
	{
	case ShapeType::Circle:
		Debug::DrawDebugCircle(this->GetLevel(), _position, shape->GetSizeData().radius * std::max(_scale.x, _scale.y), 15, Color::Green());
		break;
	case ShapeType::Rectangle:
		Debug::DrawDebugRect(this->GetLevel(), _position, FVector2(shape->GetSizeData().size.x * _scale.x, shape->GetSizeData().size.y * _scale.y), this->GetRotation(), Color::Green());
		break;
	case ShapeType::Polygon:
		Debug::DrawDebugPolygon(this->GetLevel(), Polygon2D::Transform(shape->GetOutline().points, _position, this->GetRotation(), _scale), Color::Green());
		break;
	}
#endif
}

template <typename Base>
std::string Krampus::BasicImage<Base>::ToString() const
{
	if (GetShapeType() == ShapeType::Polygon) return this->name + " -> Polygon = " + shape->GetPolygonName();
	return this->name + " -> Size/Radius = " + (GetShapeType() == ShapeType::Rectangle ? GetSize().ToString() : std::to_string(GetRadius()));
}

template class Krampus::BasicImage<Krampus::Component>;
template class Krampus::BasicImage<Krampus::WidgetComponent>;
