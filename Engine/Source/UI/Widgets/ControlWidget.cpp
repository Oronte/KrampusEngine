#include "ControlWidget.h"
#include "GameFramework/Level.h"
#include "Managers/CameraManager.h"
#include "Managers/InputManager.h"
#include "Managers/FontManager.h"
#include <cmath>

Krampus::ControlWidget::ControlWidget(Level* _level, const FVector2& _size)
	: Widget(_level), size(_size)
{
	name = NAME_OF(ControlWidget);
	SetZOrder(ZOrder::Actors);
	M_FONT.LoadFont(font, "_internal/DefaultFont", FontExtensionType::OTF);
}

Krampus::FRect Krampus::ControlWidget::GetRect() const
{
	const FVector2 _scale = GetScale();
	const FVector2 _size(size.x * std::abs(_scale.x), size.y * std::abs(_scale.y));
	return FRect(GetPosition() - _size / 2.f, _size);
}

float Krampus::ControlWidget::UniformScale() const
{
	const FVector2 _scale = GetScale();
	return std::max(0.01f, (std::abs(_scale.x) + std::abs(_scale.y)) * 0.5f);
}

void Krampus::ControlWidget::SetZOrder(const uint8_t& _zOrder)
{
	const uint8_t _old = GetZOrder();
	IDrawable::SetZOrder(_zOrder);
	if (registered) GetLevel()->GetCameraManager().ChangeScreenZOrder(this, _old);	// never puts back a destroyed widget
}

void Krampus::ControlWidget::SetZOrder(const ZOrder& _zOrder)
{
	SetZOrder(static_cast<uint8_t>(_zOrder));
}

void Krampus::ControlWidget::Construct()
{
	Widget::Construct();
	if (!registered)
	{
		GetLevel()->GetCameraManager().AddToScreen(this);
		registered = true;
	}

	InputManager* _input = GetWorld()->GetInputManager();
	mouseMoveHandle = _input->MouseMovedScreen.AddListener([this](IVector2 _pixel)
		{
			// Widgets live in the HUD view (not in window pixels), whatever the window size is.
			mouse = GetWorld()->GetWindowRef().MapPixelToCoords(_pixel, GetLevel()->GetHUD().GetView());
			hovered = IsUnderMouse();
			if (AcceptsInput()) OnMouseMove();
		});
	mousePressHandle = _input->MouseLeftClick.onPress.AddListener([this]()
		{
			hovered = IsUnderMouse();	// the widget may have moved under a still mouse
			if (AcceptsInput()) OnMouseDown();
		});
	mouseReleaseHandle = _input->MouseLeftClick.onRelease.AddListener([this]()
		{
			if (AcceptsInput()) OnMouseUp();
		});
	wheelHandle = _input->MouseWheelScroll.AddListener([this](Float _delta)
		{
			if (AcceptsInput() && IsUnderMouse()) OnWheel(_delta);
		});
	textHandle = _input->TextEntered.AddListener([this](std::uint32_t _character)
		{
			if (AcceptsInput()) OnText(_character);
		});
}

void Krampus::ControlWidget::Deconstruct()
{
	Widget::Deconstruct();
	mouseMoveHandle.Disconnect();
	mousePressHandle.Disconnect();
	mouseReleaseHandle.Disconnect();
	wheelHandle.Disconnect();
	textHandle.Disconnect();
	if (registered)
	{
		GetLevel()->GetCameraManager().RemoveToScreen(this);
		registered = false;
	}
}

void Krampus::ControlWidget::Draw(Window& _window)
{
	if (!IsActive()) return;
	Layout();
	const float _thickness = static_cast<float>(outlineThickness) * UniformScale();
	if (backgroundColor.a > 0 || (outlineColor.a > 0 && _thickness > 0.f))
		FillRect(_window, GetRect(), backgroundColor, outlineColor, _thickness);
	DrawControl(_window);
}

bool Krampus::ControlWidget::IsUnderMouse() const
{
	if (!GetRect().Contains(mouse)) return false;
	const std::optional<FRect> _clip = GetClip();
	return !_clip || _clip->Contains(mouse);
}

bool Krampus::ControlWidget::AcceptsInput() const
{
	if (!IsActive() || IsPendingDestroy()) return false;
	const std::optional<FRect> _clip = GetClip();
	return !_clip || (_clip->GetSize().x > 0.f && _clip->GetSize().y > 0.f);
}

// ---- Drawing

void Krampus::ControlWidget::FillRect(Window& _window, const FRect& _rect, const Color& _fill, const Color& _outline, const float& _thickness)
{
	sf::RectangleShape _shape(sf::Vector2f(_rect.GetSize()));
	_shape.setPosition(sf::Vector2f(_rect.GetPosition()));
	_shape.setFillColor(_fill);
	if (_thickness > 0.f && _outline.a > 0)
	{
		_shape.setOutlineColor(_outline);
		_shape.setOutlineThickness(-_thickness);	// inside the rectangle: the widget does not grow
	}
	_window.Draw(_shape);
}

void Krampus::ControlWidget::FillCircle(Window& _window, const FVector2& _center, const float& _radius, const Color& _fill, const Color& _outline, const float& _thickness)
{
	sf::CircleShape _shape(std::max(0.5f, _radius), 24);
	_shape.setOrigin({ _shape.getRadius(), _shape.getRadius() });
	_shape.setPosition(sf::Vector2f(_center));
	_shape.setFillColor(_fill);
	if (_thickness > 0.f && _outline.a > 0)
	{
		_shape.setOutlineColor(_outline);
		_shape.setOutlineThickness(-_thickness);
	}
	_window.Draw(_shape);
}

float Krampus::ControlWidget::TextWidth(const sf::String& _text, const unsigned& _characterSize) const
{
	const sf::Text _sfText(font, _text, _characterSize);
	return _sfText.getLocalBounds().size.x;
}

float Krampus::ControlWidget::DrawLabel(Window& _window, const sf::String& _text, const FVector2& _anchor, const unsigned& _characterSize, const Color& _color, const float& _alignX) const
{
	sf::Text _sfText(font, _text, std::max(1u, _characterSize));
	_sfText.setFillColor(_color);
	// Centered on the middle of a capital letter, whatever the string holds: texts with and without "g" or "p" stay aligned.
	const float _width = _sfText.getLocalBounds().size.x;
	const sf::FloatRect _capital = sf::Text(font, "H", _sfText.getCharacterSize()).getLocalBounds();
	_sfText.setPosition({ std::round(_anchor.x - _width * _alignX - _sfText.getLocalBounds().position.x), std::round(_anchor.y - _capital.position.y - _capital.size.y * 0.5f) });
	_window.Draw(_sfText);
	return _width;
}

void Krampus::ControlWidget::DrawClipped(Window& _window, const FRect& _rect, const std::function<void()>& _draw)
{
	sf::RenderWindow& _target = _window.GetRenderWindow();
	const sf::View _view = _target.getView();
	const std::optional<sf::View> _clipped = ClipView(_view, _rect);
	if (!_clipped) return;
	_target.setView(*_clipped);
	_draw();
	_target.setView(_view);
}

// ---- Text

sf::String Krampus::FromUtf8(const std::string& _text)
{
	return sf::String::fromUtf8(_text.begin(), _text.end());
}

std::string Krampus::ToUtf8(const sf::String& _text)
{
	const auto _bytes = _text.toUtf8();
	return std::string(_bytes.begin(), _bytes.end());
}
