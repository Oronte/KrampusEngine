#include "LayoutWidgets.h"
#include <cmath>

// ---- Panel

Krampus::PanelWidget::PanelWidget(Level* _level, const FVector2& _size)
	: ControlWidget(_level, _size)
{
	name = NAME_OF(PanelWidget);
	SetZOrder(static_cast<uint8_t>(45));	// behind the images and labels of its children (50)
}

std::vector<Krampus::Widget*> Krampus::PanelWidget::LaidOutChildren() const
{
	std::vector<Widget*> _children;
	for (Widget* _child : GetChildren())
		if (_child->IsActive() && !_child->IsPendingDestroy()) _children.push_back(_child);
	return _children;
}

void Krampus::PanelWidget::PlaceAt(Widget* _child, const FVector2& _topLeft)
{
	const FVector2 _offset = _topLeft - _child->GetBounds().GetPosition();
	if (std::abs(_offset.x) < 0.01f && std::abs(_offset.y) < 0.01f) return;	// a setter always broadcasts: not every frame
	_child->SetPosition(_child->GetPosition() + _offset);
}

void Krampus::PanelWidget::FitTo(const FVector2& _content)
{
	const FVector2 _scale = GetScale();
	const float _padding = static_cast<float>(padding) * UniformScale() * 2.f;
	const FVector2 _fitted((_content.x + _padding) / std::max(0.01f, std::abs(_scale.x)), (_content.y + _padding) / std::max(0.01f, std::abs(_scale.y)));
	if (std::abs(_fitted.x - size.x) > 0.01f || std::abs(_fitted.y - size.y) > 0.01f) size = _fitted;
}

// ---- Canvas

Krampus::CanvasWidget::CanvasWidget(Level* _level)
	: PanelWidget(_level, FVector2(400.f, 300.f))
{
	name = NAME_OF(CanvasWidget);
	backgroundColor = Color(20, 20, 28, 200);
	outlineColor = Color(255, 255, 255, 40);
	outlineThickness = 1.f;
}

// ---- Boxes

Krampus::BoxWidget::BoxWidget(Level* _level, const bool& _vertical)
	: PanelWidget(_level, _vertical ? FVector2(220.f, 300.f) : FVector2(400.f, 80.f))
{
	name = NAME_OF(BoxWidget);
	vertical = _vertical;
	backgroundColor = Color(20, 20, 28, 140);
}

void Krampus::BoxWidget::Layout()
{
	const std::vector<Widget*> _children = LaidOutChildren();
	const float _scale = UniformScale();
	const float _padding = static_cast<float>(padding) * _scale, _spacing = static_cast<float>(spacing) * _scale;

	// The size of the row of children, along and across the box.
	float _along = 0.f, _across = 0.f;
	for (Widget* _child : _children)
	{
		const FVector2 _size = _child->GetBounds().GetSize();
		_along += vertical ? _size.y : _size.x;
		_across = std::max(_across, static_cast<float>(vertical ? _size.x : _size.y));
	}
	if (!_children.empty()) _along += _spacing * static_cast<float>(_children.size() - 1);
	if (fitContent) FitTo(vertical ? FVector2(_across, _along) : FVector2(_along, _across));

	const FRect _rect = GetRect();
	const float _start = vertical ? _rect.GetPosition().y : _rect.GetPosition().x;
	const float _length = vertical ? _rect.GetSize().y : _rect.GetSize().x;
	const float _free = std::max(0.f, _length - 2.f * _padding - _along);
	float _cursor = _start + _padding + (justify == 1 ? _free * 0.5f : (justify == 2 ? _free : 0.f));

	for (Widget* _child : _children)
	{
		const FVector2 _size = _child->GetBounds().GetSize();
		const float _crossStart = vertical ? _rect.GetPosition().x : _rect.GetPosition().y;
		const float _crossLength = vertical ? _rect.GetSize().x : _rect.GetSize().y;
		const float _childCross = vertical ? _size.x : _size.y;
		const float _cross = alignment == 0 ? _crossStart + _padding
			: (alignment == 2 ? _crossStart + _crossLength - _padding - _childCross : _crossStart + (_crossLength - _childCross) * 0.5f);
		PlaceAt(_child, vertical ? FVector2(_cross, _cursor) : FVector2(_cursor, _cross));
		_cursor += (vertical ? _size.y : _size.x) + _spacing;
	}
}

Krampus::HorizontalBoxWidget::HorizontalBoxWidget(Level* _level)
	: BoxWidget(_level, false)
{
	name = NAME_OF(HorizontalBoxWidget);
}

Krampus::VerticalBoxWidget::VerticalBoxWidget(Level* _level)
	: BoxWidget(_level, true)
{
	name = NAME_OF(VerticalBoxWidget);
}

// ---- Grid

Krampus::GridWidget::GridWidget(Level* _level)
	: PanelWidget(_level, FVector2(360.f, 360.f))
{
	name = NAME_OF(GridWidget);
	backgroundColor = Color(20, 20, 28, 140);
}

void Krampus::GridWidget::Layout()
{
	const std::vector<Widget*> _children = LaidOutChildren();
	const float _scale = UniformScale();
	const float _padding = static_cast<float>(padding) * _scale, _spacing = static_cast<float>(spacing) * _scale;
	const int _columns = std::max(1, static_cast<int>(columns));

	// A cell: the size set, or the biggest child on each axis where it is 0.
	FVector2 _cell(static_cast<float>(cellSize.x) * _scale, static_cast<float>(cellSize.y) * _scale);
	if (cellSize.x <= 0.f || cellSize.y <= 0.f)
		for (Widget* _child : _children)
		{
			const FVector2 _size = _child->GetBounds().GetSize();
			if (cellSize.x <= 0.f) _cell.x = std::max(static_cast<float>(_cell.x), static_cast<float>(_size.x));
			if (cellSize.y <= 0.f) _cell.y = std::max(static_cast<float>(_cell.y), static_cast<float>(_size.y));
		}

	const int _rows = (static_cast<int>(_children.size()) + _columns - 1) / _columns;
	const int _usedColumns = std::min(_columns, static_cast<int>(_children.size()));
	if (fitContent)
		FitTo(FVector2(_cell.x * _usedColumns + _spacing * std::max(0, _usedColumns - 1), _cell.y * _rows + _spacing * std::max(0, _rows - 1)));

	const FVector2 _origin = GetRect().GetPosition() + FVector2(_padding, _padding);
	for (size_t _i = 0; _i < _children.size(); ++_i)
	{
		const int _column = static_cast<int>(_i) % _columns, _row = static_cast<int>(_i) / _columns;
		const FVector2 _size = _children[_i]->GetBounds().GetSize();
		const FVector2 _cellStart = _origin + FVector2((_cell.x + _spacing) * _column, (_cell.y + _spacing) * _row);
		PlaceAt(_children[_i], _cellStart + (_cell - _size) / 2.f);
	}
}

// ---- Scroll box

Krampus::ScrollBoxWidget::ScrollBoxWidget(Level* _level)
	: PanelWidget(_level, FVector2(260.f, 320.f))
{
	name = NAME_OF(ScrollBoxWidget);
	backgroundColor = Color(20, 20, 28, 200);
	outlineColor = Color(255, 255, 255, 40);
	outlineThickness = 1.f;
	alignment = 0;
	++clippingWidgets;
}

Krampus::ScrollBoxWidget::~ScrollBoxWidget()
{
	--clippingWidgets;
}

void Krampus::ScrollBoxWidget::Layout()
{
	const std::vector<Widget*> _children = LaidOutChildren();
	const float _scale = UniformScale();
	const float _padding = static_cast<float>(padding) * _scale, _spacing = static_cast<float>(spacing) * _scale;

	contentHeight = 0.f;
	for (Widget* _child : _children) contentHeight += _child->GetBounds().GetSize().y;
	if (!_children.empty()) contentHeight += _spacing * static_cast<float>(_children.size() - 1);

	const FRect _rect = GetRect();
	maxScroll = std::max(0.f, (contentHeight + 2.f * _padding - _rect.GetSize().y) / _scale);
	scroll = std::clamp(static_cast<float>(scroll), 0.f, maxScroll);

	float _y = _rect.GetPosition().y + _padding - static_cast<float>(scroll) * _scale;
	const float _barSpace = showScrollBar && maxScroll > 0.f ? 10.f * _scale : 0.f;	// the children do not go under the bar
	for (Widget* _child : _children)
	{
		const FVector2 _size = _child->GetBounds().GetSize();
		const float _width = _rect.GetSize().x - _barSpace;
		const float _x = alignment == 0 ? _rect.GetPosition().x + _padding
			: (alignment == 2 ? _rect.GetPosition().x + _width - _padding - _size.x : _rect.GetPosition().x + (_width - _size.x) * 0.5f);
		PlaceAt(_child, FVector2(_x, _y));
		_y += _size.y + _spacing;
	}
}

void Krampus::ScrollBoxWidget::DrawControl(Window& _window)
{
	if (!showScrollBar || maxScroll <= 0.f) return;
	const FRect _rect = GetRect();
	const float _scale = UniformScale();
	const float _track = _rect.GetSize().y - 8.f * _scale;
	const float _visible = _rect.GetSize().y / std::max(1.f, contentHeight + 2.f * static_cast<float>(padding) * _scale);
	const float _thumb = std::max(20.f * _scale, _track * std::min(1.f, _visible));
	const float _y = _rect.GetPosition().y + 4.f * _scale + (_track - _thumb) * (static_cast<float>(scroll) / maxScroll);
	FillRect(_window, FRect(FVector2(_rect.GetPosition().x + _rect.GetSize().x - 8.f * _scale, _y), FVector2(4.f * _scale, _thumb)), scrollBarColor);
}

void Krampus::ScrollBoxWidget::OnWheel(const float& _delta)
{
	scroll = std::clamp(static_cast<float>(scroll) - _delta * static_cast<float>(scrollSpeed), 0.f, maxScroll);	// the wheel up shows what is above
}

// ---- Spacer

Krampus::SpacerWidget::SpacerWidget(Level* _level)
	: ControlWidget(_level, FVector2(40.f, 40.f))
{
	name = NAME_OF(SpacerWidget);
}
