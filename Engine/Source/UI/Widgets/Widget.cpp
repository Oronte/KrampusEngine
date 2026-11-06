#include "Widget.h"
#include "GameFramework/Level.h"
#include "WidgetComponents/ImageWidgetComponent.h"
#include "WidgetComponents/LabelWidgetComponent.h"

Krampus::Widget::Widget(Level* _level)
	: SceneObject(_level)
{
	name = NAME_OF(Widget);
}

void Krampus::Widget::OnDestroy()
{
	level->GetHUD().MarkForDeleteWidget(this);
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
	if (!_any) return FRect(GetPosition(), FVector2());
	return FRect(FVector2(_left, _top), FVector2(_right - _left, _bottom - _top));
}

std::optional<Krampus::FRect> Krampus::Widget::GetClip() const
{
	std::optional<FRect> _clip;
	for (const Widget* _above = GetParent(); _above; _above = _above->GetParent())
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
