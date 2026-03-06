#include "Debug.h"
#include "actors/Actor.h"
#include "GameFramework/Level.h"
#include "Graphics/Window.h"

void Krampus::Debug::DrawDebugCircle(Actor* _context, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
	DrawDebugCircle(_context->GetLevel(), _position, _radius, _pointCount, _color);
}

void Krampus::Debug::DrawDebugCircle(Level* _level, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
#ifdef DEBUG
	std::shared_ptr<Vertices> _vert = std::make_shared<Vertices>(PrimitiveType::LineStrip);

	FVector2 _initialPoint = _position + FVector2(0.f, _radius);

	const float& _rotateRad = FMath::pi * 2.0f / _pointCount;
	const int& _pointCountPlusOne = _pointCount + 1;
	for (int _index = 0; _index < _pointCountPlusOne; _index++)
	{
		_vert->Append(_initialPoint.RotateAround(_position, _rotateRad * _index), _color);
	}

	_level->GetCameraManagerRef().debugs.push_back(_vert);
#endif
}

void Krampus::Debug::DrawFillCircle(Actor* _context, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
	DrawFillCircle(_context->GetLevel(), _position, _radius, _pointCount, _color);
}

void Krampus::Debug::DrawFillCircle(Level* _level, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
#ifdef DEBUG
	std::shared_ptr<Vertices> _vert = std::make_shared<Vertices>(PrimitiveType::TriangleStrip);

	FVector2 _initialPoint = _position + FVector2(0.f, _radius);

	const float& _rotateRad = FMath::pi * 2.0f / _pointCount;
	const int& _pointCountPlusOne = _pointCount + 1;
	for (int _index = 0; _index < _pointCountPlusOne; _index++)
	{
		_vert->Append(_initialPoint.RotateAround(_position, _rotateRad * _index), _color);
		_vert->Append(_position, _color);
	}

	_level->GetCameraManagerRef().debugs.push_back(_vert); 
#endif
}

void Krampus::Debug::DrawPointCircle(Actor* _context, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
	DrawPointCircle(_context->GetLevel(), _position, _radius, _pointCount, _color);
}

void Krampus::Debug::DrawPointCircle(Level* _level, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color)
{
#ifdef DEBUG
	std::shared_ptr<Vertices> _vert = std::make_shared<Vertices>(PrimitiveType::Points);

	FVector2 _initialPoint = _position + FVector2(0.f, _radius);

	const float& _rotateRad = FMath::pi * 2.0f / _pointCount;
	const int& _pointCountPlusOne = _pointCount + 1;
	for (int _index = 0; _index < _pointCountPlusOne; _index++)
	{
		_vert->Append(_initialPoint.RotateAround(_position, _rotateRad * _index), _color);
	}
	_vert->Append(_position, _color);

	_level->GetCameraManagerRef().debugs.push_back(_vert);
#endif
}

void Krampus::Debug::DrawDebugRect(Actor* _context, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
	DrawDebugRect(_context->GetLevel(), _position, _size, _rotation, _color);
}

void Krampus::Debug::DrawDebugRect(Actor* _context, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawDebugRect(_context->GetLevel(), _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawDebugRect(Level* _level, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
#ifdef DEBUG
	std::shared_ptr<Vertices> _vert = std::make_shared<Vertices>(PrimitiveType::LineStrip);

	const FVector2& _initialPos = (_position - _size / 2.f).RotateAround(_position, _rotation.ToRadians());

	_vert->Append(_initialPos, _color);
	_vert->Append(FVector2(_position.x + _size.x / 2.f, _position.y - _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert->Append((_position + _size / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert->Append(FVector2(_position.x - _size.x / 2.f, _position.y + _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert->Append(_initialPos, _color);

	_level->GetCameraManagerRef().debugs.push_back(_vert);
#endif
}

void Krampus::Debug::DrawDebugRect(Level* _level, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawDebugRect(_level, _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawFillRect(Actor* _context, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawFillRect(_context->GetLevel(), _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawFillRect(Level* _level, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawFillRect(_level, _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawFillRect(Actor* _context, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
	DrawFillRect(_context->GetLevel(), _position, _size, _rotation, _color);
}

void Krampus::Debug::DrawFillRect(Level* _level, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
#ifdef DEBUG
	std::shared_ptr<Vertices> _vert = std::make_shared<Vertices>(PrimitiveType::TriangleStrip);

	const FVector2& _initialPos = (_position - _size / 2.f).RotateAround(_position, _rotation.ToRadians());

	_vert->Append(_initialPos, _color);
	_vert->Append(FVector2(_position.x + _size.x / 2.f, _position.y - _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert->Append((_position + _size / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert->Append(FVector2(_position.x - _size.x / 2.f, _position.y + _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert->Append(_initialPos, _color);

	_level->GetCameraManagerRef().debugs.push_back(_vert);
#endif
}

void Krampus::Debug::DrawPointRect(Actor* _context, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawPointRect(_context->GetLevel(), _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawPointRect(Level* _level, const FRect& _rect, const Angle& _rotation, const Color& _color)
{
	DrawPointRect(_level, _rect.GetPosition(), _rect.GetSize(), _rotation, _color);
}

void Krampus::Debug::DrawPointRect(Actor* _context, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
	DrawPointRect(_context->GetLevel(), _position, _size, _rotation, _color);
}

void Krampus::Debug::DrawPointRect(Level* _level, const FVector2& _position, const FVector2& _size, const Angle& _rotation, const Color& _color)
{
#ifdef DEBUG
	std::shared_ptr<Vertices> _vert = std::make_shared<Vertices>(PrimitiveType::Points);

	const FVector2& _initialPos = (_position - _size / 2.f).RotateAround(_position, _rotation.ToRadians());

	_vert->Append(_initialPos, _color);
	_vert->Append(FVector2(_position.x + _size.x / 2.f, _position.y - _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert->Append((_position + _size / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert->Append(FVector2(_position.x - _size.x / 2.f, _position.y + _size.y / 2.f).RotateAround(_position, _rotation.ToRadians()), _color);
	_vert->Append(_initialPos, _color);
	_vert->Append(_position, _color);

	_level->GetCameraManagerRef().debugs.push_back(_vert);
#endif
}

void Krampus::Debug::DrawDebugPoint(Actor* _context, const FVector2& _position, const float& _thickness, const Color& _color)
{
	DrawDebugPoint(_context->GetLevel(), _position, _thickness, _color);
}

void Krampus::Debug::DrawDebugPoint(Level* _level, const FVector2& _position, const float& _thickness, const Color& _color)
{
	DrawFillCircle(_level, _position, _thickness, 30, _color);
}

void Krampus::Debug::DrawDebugLine(Actor* _context, const FVector2& _start, const FVector2& _direction, const float _length, const float _thickness, const Color& _color)
{
	DrawDebugLine(_context->GetLevel(), _start, _direction, _length, _thickness, _color);
}

void Krampus::Debug::DrawDebugLine(Level* _level, const FVector2& _start, const FVector2& _direction, const float _length, const float _thickness, const Color& _color)
{
	const FVector2& _pos = _start + _direction * (_length * 0.5f);
	DrawFillRect(_level, _pos, FVector2(_length, _thickness), FVector2::AngleBetweenRadians(FVector2::Right(), _direction), _color);
}
