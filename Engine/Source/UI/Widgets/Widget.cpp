#include "Widget.h"
#include "GameFramework/Level.h"
#include "Managers/TimerManager.h"
#include "WidgetComponents/ImageWidgetComponent.h"
#include "WidgetComponents/LabelWidgetComponent.h"

Krampus::Engine* Krampus::Widget::GetWorld() const
{
	return level->GetWorld();
}

Float Krampus::Widget::GetDeltaTime() const
{
	return level->GetTimerManager().GetDeltaTime();
}

Krampus::Widget::Widget(Level* _level)
{
	name = NAME_OF(Widget);
	level = _level;
}

void Krampus::Widget::SetActive(const Bool& _status)
{
	IGameObject::SetActive(_status);

	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		_component->SetActive(_status);
	}
}

Krampus::Timer* Krampus::Widget::CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop, const Bool& _startRunning)
{
	return level->GetTimerManager().CreateTimer(_callback, _duration, _isLoop, _startRunning);
}

void Krampus::Widget::Construct()
{
	// The components created in the constructor were waiting for this (CreateComponent): each one is constructed once.
	if (constructed) return;
	constructed = true;
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		_component->Construct();
	}
}

void Krampus::Widget::Deconstruct()
{
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		_component->Deconstruct();
	}
}

void Krampus::Widget::BeginPlay()
{
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		_component->BeginPlay();
	}
}

void Krampus::Widget::Tick(const Float& _deltaTime)
{
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		if (_component->IsActive()) 
			_component->Tick(_deltaTime);
	}
}

void Krampus::Widget::BeginDestroy()
{
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		_component->BeginDestroy();
	}
}

void Krampus::Widget::Destroy()
{
	if (pendingDestroy) return;	// a second Destroy would deconstruct the components twice
	pendingDestroy = true;
	Deconstruct();
	level->GetHUD().MarkForDeleteWidget(this);

	// Out of the hierarchy: the parent would move a deleted widget. The children stay in the HUD, without a parent.
	if (parent) parent->RemoveChild(this);
	for (Widget* _child : children)
	{
		_child->parent = nullptr;
		_child->localTransform = _child->transform;
	}
	children.clear();
}

std::string Krampus::Widget::ToString() const
{
	return name + " -> Transform = " + transform.ToString();
}

void Krampus::Widget::SetPosition(const FVector2& _newPosition)
{
	transform.position = _newPosition;
	WorldChanged();
	onMove.Broadcast(_newPosition);		// even when it did not change: a setter always tells
}

Krampus::FVector2 Krampus::Widget::GetPosition() const
{
	return transform.position;
}

void Krampus::Widget::SetRotation(const Angle& _newRotation)
{
	transform.rotation = _newRotation;
	WorldChanged();
	onRotate.Broadcast(_newRotation);
}

Krampus::Angle Krampus::Widget::GetRotation() const
{
	return transform.rotation;
}

void Krampus::Widget::SetScale(const FVector2& _newScale)
{
	transform.scale = _newScale;
	WorldChanged();
	onScale.Broadcast(_newScale);
}

void Krampus::Widget::SetLocalTransform(const Transform& _local)
{
	localTransform = _local;
	const Transform _old = transform;
	FollowParent();
	Propagate(_old);
}

void Krampus::Widget::WorldChanged()
{
	localTransform = parent ? RelativeTransform(parent->transform, transform) : transform;
	const Transform _same = transform;
	Propagate(_same);	// only the children: nothing differs for this one
}

void Krampus::Widget::FollowParent()
{
	transform = parent ? ComposeTransform(parent->transform, localTransform) : localTransform;
}

void Krampus::Widget::Propagate(const Transform& _old)
{
	for (Widget* _child : children)
	{
		const Transform _childOld = _child->transform;
		_child->FollowParent();
		_child->Propagate(_childOld);
	}
	if (transform.position.x != _old.position.x || transform.position.y != _old.position.y) onMove.Broadcast(transform.position);
	if (transform.rotation != _old.rotation) onRotate.Broadcast(transform.rotation);
	if (transform.scale.x != _old.scale.x || transform.scale.y != _old.scale.y) onScale.Broadcast(transform.scale);
}

Krampus::FVector2 Krampus::Widget::GetScale() const
{
	return transform.scale;
}

Krampus::FRect Krampus::Widget::GetBounds() const
{
	bool _any = false;
	float _left = 0.f, _top = 0.f, _right = 0.f, _bottom = 0.f;
	const auto _add = [&](const FRect& _rect)
	{
		const FVector2 _min = _rect.GetPosition(), _max = _rect.GetPosition() + _rect.GetSize();
		if (!_any) { _left = _min.x; _top = _min.y; _right = _max.x; _bottom = _max.y; _any = true; return; }
		_left = std::min(_left, static_cast<float>(_min.x)); _top = std::min(_top, static_cast<float>(_min.y));
		_right = std::max(_right, static_cast<float>(_max.x)); _bottom = std::max(_bottom, static_cast<float>(_max.y));
	};
	for (const std::unique_ptr<WidgetComponent>& _component : components)
	{
		if (const ImageWidgetComponent* _image = dynamic_cast<const ImageWidgetComponent*>(_component.get()))
			_add(_image->GetShapeObject()->GetShape()->GetGlobalBounds());
		else if (const LabelWidgetComponent* _label = dynamic_cast<const LabelWidgetComponent*>(_component.get()))
			_add(_label->GetGlobalBounds());
	}
	if (!_any) return FRect(transform.position, FVector2());
	return FRect(FVector2(_left, _top), FVector2(_right - _left, _bottom - _top));
}

std::optional<Krampus::FRect> Krampus::Widget::GetClip() const
{
	std::optional<FRect> _clip;
	for (const Widget* _above = parent; _above; _above = _above->parent)
	{
		const std::optional<FRect> _rect = _above->GetChildClip();
		if (!_rect) continue;
		if (!_clip) { _clip = _rect; continue; }
		const std::optional<FRect> _both = _clip->FindIntersection(*_rect);
		_clip = _both ? *_both : FRect(_clip->GetPosition(), FVector2());	// nothing left: an empty rectangle
	}
	return _clip;
}

std::optional<sf::View> Krampus::Widget::ClipView(const sf::View& _base, const FRect& _rect)
{
	const FRect _shown(FVector2(_base.getCenter() - _base.getSize() / 2.f), FVector2(_base.getSize()));
	const std::optional<FRect> _inside = _shown.FindIntersection(_rect);
	if (!_inside || _inside->GetSize().x <= 0.f || _inside->GetSize().y <= 0.f || _shown.GetSize().x <= 0.f || _shown.GetSize().y <= 0.f) return std::nullopt;

	// The part of the window the view draws in shrinks like the rectangle it shows.
	const sf::FloatRect _viewport = _base.getViewport();
	const sf::Vector2f _from = sf::Vector2f(_inside->GetPosition() - _shown.GetPosition());
	const sf::Vector2f _shownSize = sf::Vector2f(_shown.GetSize());
	sf::View _view(sf::Vector2f(_inside->GetCenter()), sf::Vector2f(_inside->GetSize()));
	_view.setViewport(sf::FloatRect(
		{ _viewport.position.x + _from.x / _shownSize.x * _viewport.size.x, _viewport.position.y + _from.y / _shownSize.y * _viewport.size.y },
		{ _inside->GetSize().x / _shownSize.x * _viewport.size.x, _inside->GetSize().y / _shownSize.y * _viewport.size.y }));
	return _view;
}
