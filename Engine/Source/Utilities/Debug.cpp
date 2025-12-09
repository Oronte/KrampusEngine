#include "Debug.h"
#include "actors/Actor.h"
#include "Graphics/Window/Window.h"
#include "GameFramework/Level.h"
#include "Graphics/Window/MainWindow.h"

void Krampus::Debug::DrawDebugCircle(Actor* _context, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
	DrawDebugCircle(MAIN_WINDOW, _position, _radius, _pointCount, _color);
}

void Krampus::Debug::DrawDebugCircle(Window& _window, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
	Vertices _vert = Vertices(PrimitiveType::LineStrip);

	FVector2 _initialPoint = _position + FVector2(0.f, _radius);

	const float& _rotateRad = FMath::pi * 2.0f / _pointCount;
	const int& _pointCountPlusOne = _pointCount + 1;
	for (int _index = 0; _index < _pointCountPlusOne; _index++)
	{
		_vert.Append(_initialPoint.RotateAround(_position, _rotateRad * _index), _color);
	}

	_window.Draw(_vert);
}

void Krampus::Debug::DrawFillCircle(Actor* _context, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
	DrawFillCircle(MAIN_WINDOW, _position, _radius, _pointCount, _color);
}

void Krampus::Debug::DrawFillCircle(Window& _window, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
	Vertices _vert = Vertices(PrimitiveType::TriangleStrip);

	FVector2 _initialPoint = _position + FVector2(0.f, _radius);

	const float& _rotateRad = FMath::pi * 2.0f / _pointCount;
	const int& _pointCountPlusOne = _pointCount + 1;
	for (int _index = 0; _index < _pointCountPlusOne; _index++)
	{
		_vert.Append(_initialPoint.RotateAround(_position, _rotateRad * _index), _color);
		_vert.Append(_position, _color);
	}

	_window.Draw(_vert);
}

void Krampus::Debug::DrawPointCircle(Actor* _context, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
	DrawPointCircle(MAIN_WINDOW, _position, _radius, _pointCount, _color);
}

void Krampus::Debug::DrawPointCircle(Window& _window, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
	Vertices _vert = Vertices(PrimitiveType::Points);

	FVector2 _initialPoint = _position + FVector2(0.f, _radius);

	const float& _rotateRad = FMath::pi * 2.0f / _pointCount;
	const int& _pointCountPlusOne = _pointCount + 1;
	for (int _index = 0; _index < _pointCountPlusOne; _index++)
	{
		_vert.Append(_initialPoint.RotateAround(_position, _rotateRad * _index), _color);
	}
	_vert.Append(_position, _color);

	_window.Draw(_vert);
}

void Krampus::Debug::DrawDebugRect(Actor* _context, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
	DrawDebugRect(MAIN_WINDOW, _position, _size, _rotation, _color);
}

void Krampus::Debug::DrawDebugRect(Actor* _context, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawDebugRect(MAIN_WINDOW, _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawDebugRect(Window& _window, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
	Vertices _vert = Vertices(PrimitiveType::LineStrip);

	const FVector2& _initialPos = (_position - _size / 2.f).RotateAround(_position, _rotation.ToRadians());

	_vert.Append(_initialPos, _color);
	_vert.Append(FVector2(_position.x + _size.x / 2.f, _position.y - _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert.Append((_position + _size / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert.Append(FVector2(_position.x - _size.x / 2.f, _position.y + _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert.Append(_initialPos, _color);

	_window.Draw(_vert);
}

void Krampus::Debug::DrawDebugRect(Window& _window, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawDebugRect(_window, _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawFillRect(Actor* _context, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawFillRect(MAIN_WINDOW, _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawFillRect(Window& _window, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawFillRect(_window, _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawFillRect(Actor* _context, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
	DrawFillRect(MAIN_WINDOW, _position, _size, _rotation, _color);
}

void Krampus::Debug::DrawFillRect(Window& _window, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
	Vertices _vert = Vertices(PrimitiveType::TriangleStrip);

	const FVector2& _initialPos = (_position - _size / 2.f).RotateAround(_position, _rotation.ToRadians());

	_vert.Append(_initialPos, _color);
	_vert.Append(FVector2(_position.x + _size.x / 2.f, _position.y - _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert.Append((_position + _size / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert.Append(FVector2(_position.x - _size.x / 2.f, _position.y + _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert.Append(_initialPos, _color);

	_window.Draw(_vert);
}

void Krampus::Debug::DrawPointRect(Actor* _context, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawPointRect(MAIN_WINDOW, _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawPointRect(Window& _window, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawPointRect(_window, _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawPointRect(Actor* _context, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
	DrawPointRect(MAIN_WINDOW, _position, _size, _rotation, _color);
}

void Krampus::Debug::DrawPointRect(Window& _window, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
	Vertices _vert = Vertices(PrimitiveType::Points);

	const FVector2& _initialPos = (_position - _size / 2.f).RotateAround(_position, _rotation.ToRadians());

	_vert.Append(_initialPos, _color);
	_vert.Append(FVector2(_position.x + _size.x / 2.f, _position.y - _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert.Append((_position + _size / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert.Append(FVector2(_position.x - _size.x / 2.f, _position.y + _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert.Append(_initialPos, _color);
	_vert.Append(_position, _color);

	_window.Draw(_vert);
}

void Krampus::Debug::DrawDebugPoint(Actor* _context, const FVector2& _position, const float& _thickness, const Color& _color)
{
	DrawDebugPoint(MAIN_WINDOW, _position, _thickness, _color);
}

void Krampus::Debug::DrawDebugPoint(Window& _window, const FVector2& _position, const float& _thickness, const Color& _color)
{
	DrawFillCircle(_window, _position, _thickness, 30, _color);
}
