#pragma once
#include "Graphics/Ressources/Vertices.h"
#include "Utilities/Math/Angle.h"

namespace Krampus
{

	class Actor;
	class Level;

	class Debug
	{
	public:
		// Circle
		static void DrawDebugCircle(Actor* _context, const FVector2& _position, const float& _radius, const int& _pointCount = 15, const Color& _color = Color::Magenta());
		static void DrawDebugCircle(Level* _level, const FVector2& _position, const float& _radius, const int& _pointCount = 15, const Color& _color = Color::Magenta());
		
		static void DrawFillCircle(Actor* _context, const FVector2& _position, const float& _radius, const int& _pointCount = 15, const Color& _color = Color::Magenta());
		static void DrawFillCircle(Level* _level, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color = Color::Magenta());
		
		static void DrawPointCircle(Actor* _context, const FVector2& _position, const float& _radius, const int& _pointCount = 15, const Color& _color = Color::Magenta());
		static void DrawPointCircle(Level* _level, const FVector2& _position, const float& _radius, const int& _pointCount, const Color& _color = Color::Magenta());


		// Rectangle
		static void DrawDebugRect(Actor* _context, const FVector2& _position, const FVector2& _size, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
		static void DrawDebugRect(Actor* _context, const FRect& _rect, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
		static void DrawDebugRect(Level* _level, const FVector2& _position, const FVector2& _size, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
		static void DrawDebugRect(Level* _level, const FRect& _rect, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
		
		static void DrawFillRect(Actor* _context, const FRect& _rect, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
		static void DrawFillRect(Level* _level, const FRect& _rect, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
		static void DrawFillRect(Actor* _context, const FVector2& _position, const FVector2& _size, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
		static void DrawFillRect(Level* _level, const FVector2& _position, const FVector2& _size, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
	
		static void DrawPointRect(Actor* _context, const FRect& _rect, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
		static void DrawPointRect(Level* _level, const FRect& _rect, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
		static void DrawPointRect(Actor* _context, const FVector2& _position, const FVector2& _size, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
		static void DrawPointRect(Level* _level, const FVector2& _position, const FVector2& _size, const Angle& _rotation = Angle(), const Color& _color = Color::Magenta());
	

		// Point
		static void DrawDebugPoint(Actor* _context, const FVector2& _position, const float& _thickness = 5.0f, const Color& _color = Color::Magenta());
		static void DrawDebugPoint(Level* _level, const FVector2& _position, const float& _thickness = 5.0f, const Color& _color = Color::Magenta());
	
	
		// Line
		static void DrawDebugLine(Actor* _context, const FVector2& _start, const FVector2& _direction, const float _length = 75.0f, const float _thickness = 2.5f, const Color& _color = Color::Magenta());
		static void DrawDebugLine(Level* _level, const FVector2& _start, const FVector2& _direction, const float _length = 75.0f, const float _thickness = 2.5f, const Color& _color = Color::Magenta());
	};

}

