#include "InputWidgets.h"
#include "GameFramework/Level.h"
#include <cmath>
#include <sstream>

// ---- Slider

Krampus::SliderWidget::SliderWidget(Level* _level)
	: ControlWidget(_level, FVector2(240.f, 28.f))
{
	name = NAME_OF(SliderWidget);
}

float Krampus::SliderWidget::GetRatio() const
{
	const float _range = maxValue - minValue;
	return _range == 0.f ? 0.f : std::clamp((static_cast<float>(value) - static_cast<float>(minValue)) / _range, 0.f, 1.f);
}

void Krampus::SliderWidget::SetValue(const float& _value, const bool& _notify)
{
	const float _low = std::min(static_cast<float>(minValue), static_cast<float>(maxValue));
	const float _high = std::max(static_cast<float>(minValue), static_cast<float>(maxValue));
	float _new = std::clamp(_value, _low, _high);
	const float _step = step, _min = minValue;
	if (_step > 0.f) _new = std::clamp(_min + std::round((_new - _min) / _step) * _step, _low, _high);
	if (_new == static_cast<float>(value)) return;
	value = _new;
	if (!_notify) return;
	onValueChanged.Broadcast(_new);
	onValueChangedCalls.Invoke(this);
}

void Krampus::SliderWidget::SetFromMouse()
{
	const FRect _rect = GetRect();
	const float _handle = static_cast<float>(handleSize) * UniformScale();
	// The handle stays inside the rectangle: the track is shorter by its size.
	float _ratio = 0.f;
	if (vertical)
	{
		const float _track = std::max(1.f, _rect.GetSize().y - _handle);
		_ratio = 1.f - (mouse.y - _rect.GetPosition().y - _handle * 0.5f) / _track;
	}
	else
	{
		const float _track = std::max(1.f, _rect.GetSize().x - _handle);
		_ratio = (mouse.x - _rect.GetPosition().x - _handle * 0.5f) / _track;
	}
	const float _min = minValue, _max = maxValue;
	SetValue(_min + std::clamp(_ratio, 0.f, 1.f) * (_max - _min), true);
}

void Krampus::SliderWidget::DrawControl(Window& _window)
{
	const FRect _rect = GetRect();
	const float _scale = UniformScale();
	const float _handle = static_cast<float>(handleSize) * _scale;
	const float _ratio = GetRatio();
	const FVector2 _min = _rect.GetPosition(), _size = _rect.GetSize();

	if (vertical)
	{
		const float _thickness = std::max(2.f, _size.x * 0.3f);
		const float _x = _min.x + (_size.x - _thickness) * 0.5f;
		const float _track = std::max(0.f, _size.y - _handle);
		const float _handleY = _min.y + _handle * 0.5f + _track * (1.f - _ratio);
		FillRect(_window, FRect(FVector2(_x, _min.y + _handle * 0.5f), FVector2(_thickness, _track)), trackColor);
		FillRect(_window, FRect(FVector2(_x, _handleY), FVector2(_thickness, _min.y + _handle * 0.5f + _track - _handleY)), fillColor);
		FillCircle(_window, FVector2(_min.x + _size.x * 0.5f, _handleY), _handle * 0.5f, handleColor, fillColor, dragging || hovered ? 2.f * _scale : 0.f);
	}
	else
	{
		const float _thickness = std::max(2.f, _size.y * 0.3f);
		const float _y = _min.y + (_size.y - _thickness) * 0.5f;
		const float _track = std::max(0.f, _size.x - _handle);
		const float _handleX = _min.x + _handle * 0.5f + _track * _ratio;
		FillRect(_window, FRect(FVector2(_min.x + _handle * 0.5f, _y), FVector2(_track, _thickness)), trackColor);
		FillRect(_window, FRect(FVector2(_min.x + _handle * 0.5f, _y), FVector2(_handleX - _min.x - _handle * 0.5f, _thickness)), fillColor);
		FillCircle(_window, FVector2(_handleX, _min.y + _size.y * 0.5f), _handle * 0.5f, handleColor, fillColor, dragging || hovered ? 2.f * _scale : 0.f);
	}
}

void Krampus::SliderWidget::OnMouseDown()
{
	if (!hovered) return;
	dragging = true;
	SetFromMouse();
}

void Krampus::SliderWidget::OnMouseMove()
{
	if (dragging) SetFromMouse();
}

void Krampus::SliderWidget::OnMouseUp()
{
	if (!dragging) return;
	dragging = false;
	onReleased.Broadcast(value);
}

void Krampus::SliderWidget::OnWheel(const float& _delta)
{
	const float _step = step > 0.f ? static_cast<float>(step) : (static_cast<float>(maxValue) - static_cast<float>(minValue)) / 20.f;
	SetValue(static_cast<float>(value) + _delta * _step, true);
}

// ---- Progress bar

Krampus::ProgressBarWidget::ProgressBarWidget(Level* _level)
	: ControlWidget(_level, FVector2(260.f, 26.f))
{
	name = NAME_OF(ProgressBarWidget);
	backgroundColor = Color(36, 36, 44);
	outlineColor = Color(255, 255, 255, 50);
	outlineThickness = 1.f;
}

void Krampus::ProgressBarWidget::DrawControl(Window& _window)
{
	const FRect _rect = GetRect();
	const FVector2 _min = _rect.GetPosition(), _size = _rect.GetSize();
	const float _ratio = std::clamp(static_cast<float>(value), 0.f, 1.f);
	FRect _fill;
	switch (static_cast<int>(direction))
	{
	case 1:		_fill = FRect(FVector2(_min.x + _size.x * (1.f - _ratio), _min.y), FVector2(_size.x * _ratio, _size.y)); break;
	case 2:		_fill = FRect(FVector2(_min.x, _min.y + _size.y * (1.f - _ratio)), FVector2(_size.x, _size.y * _ratio)); break;
	case 3:		_fill = FRect(_min, FVector2(_size.x, _size.y * _ratio)); break;
	default:	_fill = FRect(_min, FVector2(_size.x * _ratio, _size.y)); break;
	}
	if (_ratio > 0.f) FillRect(_window, _fill, fillColor);
	if (outlineThickness > 0.f && outlineColor.a > 0) FillRect(_window, _rect, Color(0, 0, 0, 0), outlineColor, static_cast<float>(outlineThickness) * UniformScale());	// over the fill
	if (showText)
		DrawLabel(_window, std::to_string(static_cast<int>(std::round(_ratio * 100.f))) + "%", _rect.GetCenter(),
			static_cast<unsigned>(std::max(1.f, static_cast<float>(characterSize) * UniformScale())), textColor, 0.5f);
}

// ---- Check box

Krampus::CheckBoxWidget::CheckBoxWidget(Level* _level)
	: ControlWidget(_level, FVector2(220.f, 32.f))
{
	name = NAME_OF(CheckBoxWidget);
}

void Krampus::CheckBoxWidget::SetChecked(const bool& _checked, const bool& _notify)
{
	if (static_cast<bool>(checked) == _checked) return;
	checked = _checked;

	// A radio button: the others of the group let go (they tell it too).
	if (_checked && !group.empty())
		for (CheckBoxWidget* _other : GetLevel()->GetHUD().GetAllWidgetOfClass<CheckBoxWidget>())
			if (_other != this && _other->group == group) _other->SetChecked(false, _notify);

	if (!_notify) return;
	onChanged.Broadcast(_checked);
	(_checked ? onCheckedCalls : onUncheckedCalls).Invoke(this);
}

void Krampus::CheckBoxWidget::DrawControl(Window& _window)
{
	const FRect _rect = GetRect();
	const float _scale = UniformScale();
	const float _box = std::min(static_cast<float>(_rect.GetSize().y), static_cast<float>(_rect.GetSize().x));
	const FVector2 _boxMin(_rect.GetPosition().x, _rect.GetPosition().y + (_rect.GetSize().y - _box) * 0.5f);
	const bool _radio = !group.empty();
	const Color _outline = hovered ? checkColor : Color(255, 255, 255, 60);

	if (_radio)
	{
		const FVector2 _center = _boxMin + FVector2(_box, _box) * 0.5f;
		FillCircle(_window, _center, _box * 0.5f, boxColor, _outline, 1.5f * _scale);
		if (checked) FillCircle(_window, _center, _box * 0.25f, checkColor);
	}
	else
	{
		FillRect(_window, FRect(_boxMin, FVector2(_box, _box)), boxColor, _outline, 1.5f * _scale);
		if (checked) FillRect(_window, FRect(_boxMin + FVector2(_box, _box) * 0.22f, FVector2(_box, _box) * 0.56f), checkColor);
	}
	if (!text.empty())
		DrawLabel(_window, FromUtf8(text), FVector2(_boxMin.x + _box + 10.f * _scale, _rect.GetCenter().y),
			static_cast<unsigned>(std::max(1.f, static_cast<float>(characterSize) * _scale)), textColor);
}

void Krampus::CheckBoxWidget::OnMouseDown()
{
	if (!hovered) return;
	if (!group.empty()) SetChecked(true, true);	// a radio button is only unchecked by another one
	else SetChecked(!checked, true);
}

// ---- Text entry

Krampus::TextEntryWidget::TextEntryWidget(Level* _level)
	: ControlWidget(_level, FVector2(320.f, 42.f))
{
	name = NAME_OF(TextEntryWidget);
	backgroundColor = Color(28, 28, 34);
	outlineColor = Color(255, 255, 255, 60);
	outlineThickness = 1.5f;
}

void Krampus::TextEntryWidget::SetText(const std::string& _text)
{
	value = FromUtf8(_text);
	if (maxLength > 0 && value.getSize() > static_cast<size_t>(maxLength)) value = value.substring(0, static_cast<size_t>(maxLength));
}

void Krampus::TextEntryWidget::SetFocused(const bool& _focused)
{
	focused = _focused;
	caretClock.restart();	// the caret shows at once
}

void Krampus::TextEntryWidget::Deconstruct()
{
	ControlWidget::Deconstruct();
	focused = false;
}

void Krampus::TextEntryWidget::OnMouseDown()
{
	SetFocused(hovered);	// a click elsewhere takes the keyboard away
}

void Krampus::TextEntryWidget::OnText(const std::uint32_t& _character)
{
	if (!focused) return;
	caretClock.restart();
	switch (_character)
	{
	case 8:		// Backspace
		if (value.isEmpty()) return;
		value.erase(value.getSize() - 1);
		break;
	case 13:	// Enter
	{
		const std::string _text = GetText();
		SetFocused(false);
		onSubmit.Broadcast(_text);
		onSubmitCalls.Invoke(this);
		return;
	}
	case 27:	// Escape
		SetFocused(false);
		return;
	default:
		if (_character < 32 || _character == 127) return;	// other control keys (Tab, Ctrl+...)
		if (maxLength > 0 && value.getSize() >= static_cast<size_t>(maxLength)) return;
		value += static_cast<char32_t>(_character);
		break;
	}
	const std::string _text = GetText();
	onTextChanged.Broadcast(_text);
	onTextChangedCalls.Invoke(this);
}

void Krampus::TextEntryWidget::DrawControl(Window& _window)
{
	const FRect _rect = GetRect();
	const float _scale = UniformScale();
	if (focused) FillRect(_window, _rect, Color(0, 0, 0, 0), focusColor, 2.f * _scale);

	const float _margin = 10.f * _scale;
	const FRect _inner(_rect.GetPosition() + FVector2(_margin, 0.f), FVector2(std::max(0.f, _rect.GetSize().x - 2.f * _margin), _rect.GetSize().y));
	const unsigned _characterSize = static_cast<unsigned>(std::max(1.f, static_cast<float>(characterSize) * _scale));
	const bool _empty = value.isEmpty();
	const sf::String _shown = _empty ? FromUtf8(placeholder) : (password ? sf::String(std::u32string(value.getSize(), U'*')) : value);
	const float _width = TextWidth(_shown, _characterSize);
	// A text longer than the box shows its end, where the player types.
	const float _x = (!_empty && _width > _inner.GetSize().x) ? _inner.GetPosition().x + _inner.GetSize().x - _width : _inner.GetPosition().x;
	const float _y = _rect.GetCenter().y;
	const bool _caret = focused && std::fmod(caretClock.getElapsedTime().asSeconds(), 1.f) < 0.55f;

	DrawClipped(_window, _inner, [&]()
		{
			DrawLabel(_window, _shown, FVector2(_x, _y), _characterSize, _empty ? placeholderColor : textColor);
			if (_caret)
			{
				const float _caretX = _empty ? _x : _x + _width + 2.f * _scale;
				FillRect(_window, FRect(FVector2(_caretX, _y - _characterSize * 0.55f), FVector2(std::max(1.f, 2.f * _scale), _characterSize * 1.1f)), textColor);
			}
		});
}

// ---- Dropdown

Krampus::DropdownWidget::DropdownWidget(Level* _level)
	: ControlWidget(_level, FVector2(240.f, 38.f))
{
	name = NAME_OF(DropdownWidget);
	backgroundColor = Color(36, 36, 46);
	outlineColor = Color(255, 255, 255, 60);
	outlineThickness = 1.5f;
	SetOptions("Option 1;Option 2;Option 3");
}

void Krampus::DropdownWidget::SetOptions(const std::string& _options)
{
	optionsText = _options;
	options.clear();
	std::stringstream _stream(_options);
	for (std::string _option; std::getline(_stream, _option, ';');)
		if (!_option.empty()) options.push_back(_option);
	if (!options.empty()) selected = std::clamp(static_cast<int>(selected), 0, static_cast<int>(options.size()) - 1);
}

std::string Krampus::DropdownWidget::GetSelectedText() const
{
	return selected >= 0 && selected < static_cast<int>(options.size()) ? options[selected] : std::string();
}

void Krampus::DropdownWidget::SetSelected(const int& _index, const bool& _notify)
{
	const int _new = options.empty() ? 0 : std::clamp(_index, 0, static_cast<int>(options.size()) - 1);
	if (_new == static_cast<int>(selected)) return;
	selected = _new;
	if (!_notify) return;
	onSelectionChanged.Broadcast(_new, GetSelectedText());
	onSelectionChangedCalls.Invoke(this);
}

void Krampus::DropdownWidget::SetOpen(const bool& _open)
{
	if (_open == open) return;
	open = _open;
	// Open, the list is drawn over the other widgets; closed, it goes back to its layer.
	if (open) { closedZOrder = GetZOrder(); SetZOrder(static_cast<uint8_t>(250)); }
	else SetZOrder(closedZOrder);
}

void Krampus::DropdownWidget::Deconstruct()
{
	SetOpen(false);
	ControlWidget::Deconstruct();
}

int Krampus::DropdownWidget::ItemUnderMouse() const
{
	if (!open) return -1;
	const FRect _rect = GetRect();
	const float _height = _rect.GetSize().y;
	const FRect _list(FVector2(_rect.GetPosition().x, _rect.GetPosition().y + _height), FVector2(_rect.GetSize().x, _height * options.size()));
	if (!_list.Contains(mouse)) return -1;
	return std::clamp(static_cast<int>((mouse.y - _list.GetPosition().y) / _height), 0, static_cast<int>(options.size()) - 1);
}

void Krampus::DropdownWidget::OnMouseDown()
{
	if (const int _item = ItemUnderMouse(); _item >= 0)
	{
		SetOpen(false);
		SetSelected(_item, true);
		return;
	}
	SetOpen(!open && hovered);	// a click on it opens or closes it, elsewhere closes it
}

void Krampus::DropdownWidget::DrawControl(Window& _window)
{
	const FRect _rect = GetRect();
	const float _scale = UniformScale();
	const unsigned _characterSize = static_cast<unsigned>(std::max(1.f, static_cast<float>(characterSize) * _scale));
	const float _margin = 10.f * _scale;
	const float _height = _rect.GetSize().y;

	if (hovered && !open) FillRect(_window, _rect, Color(255, 255, 255, 18));
	DrawClipped(_window, FRect(_rect.GetPosition(), FVector2(std::max(0.f, _rect.GetSize().x - _height), _height)), [&]()
		{
			DrawLabel(_window, FromUtf8(GetSelectedText()), FVector2(_rect.GetPosition().x + _margin, _rect.GetCenter().y), _characterSize, textColor);
		});

	// The arrow: down when closed, up when open.
	const FVector2 _arrow(_rect.GetPosition().x + _rect.GetSize().x - _height * 0.5f, _rect.GetCenter().y);
	const float _half = _height * 0.16f;
	sf::ConvexShape _triangle(3);
	_triangle.setPoint(0, { _arrow.x - _half, _arrow.y + (open ? _half : -_half) * 0.6f });
	_triangle.setPoint(1, { _arrow.x + _half, _arrow.y + (open ? _half : -_half) * 0.6f });
	_triangle.setPoint(2, { _arrow.x, _arrow.y + (open ? -_half : _half) * 0.6f });
	_triangle.setFillColor(textColor);
	_window.Draw(_triangle);

	if (!open) return;
	const int _hovered = ItemUnderMouse();
	for (size_t _i = 0; _i < options.size(); ++_i)
	{
		const FRect _item(FVector2(_rect.GetPosition().x, _rect.GetPosition().y + _height * (_i + 1)), FVector2(_rect.GetSize().x, _height));
		FillRect(_window, _item, static_cast<int>(_i) == _hovered ? hoverColor : itemColor, Color(255, 255, 255, 25), 1.f);
		DrawClipped(_window, _item, [&]()
			{
				const bool _current = static_cast<int>(_i) == static_cast<int>(selected) && static_cast<int>(_i) != _hovered;
				DrawLabel(_window, FromUtf8(options[_i]), FVector2(_item.GetPosition().x + _margin, _item.GetCenter().y), _characterSize,
					_current ? Color(150, 200, 255) : textColor);	// the chosen one stands out
			});
	}
}
