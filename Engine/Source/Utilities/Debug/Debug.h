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
		static void DrawDebugCircle(Actor* _context, const FVector2& _position, const Float& _radius, const Int& _pointCount = 15, const Color& _color = Color::Magenta());
		static void DrawDebugCircle(Level* _level, const FVector2& _position, const Float& _radius, const Int& _pointCount = 15, const Color& _color = Color::Magenta());
		
		static void DrawFillCircle(Actor* _context, const FVector2& _position, const Float& _radius, const Int& _pointCount = 15, const Color& _color = Color::Magenta());
		static void DrawFillCircle(Level* _level, const FVector2& _position, const Float& _radius, const Int& _pointCount, const Color& _color = Color::Magenta());
		
		static void DrawPointCircle(Actor* _context, const FVector2& _position, const Float& _radius, const Int& _pointCount = 15, const Color& _color = Color::Magenta());
		static void DrawPointCircle(Level* _level, const FVector2& _position, const Float& _radius, const Int& _pointCount, const Color& _color = Color::Magenta());


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
	

		// Outlines (lines and points) are drawn this many pixels thick on the screen, whatever the zoom: a 1 pixel line of the
		// GPU disappears when the picture is shown smaller (the editor Viewport) or seen between two pixels.
		inline static float lineThickness = 1.5f;

		// Window pixels per pixel seen: the editor shows the game picture smaller in its Viewport and sets it every frame (1: full size).
		inline static float displayScale = 1.f;

		// Segment: a line from _start to _end, as thick as lineThickness on the screen.
		static void DrawDebugSegment(Level* _level, const FVector2& _start, const FVector2& _end, const Color& _color = Color::Magenta());

		// Polygon: the outline through _points (world positions), closed.
		static void DrawDebugPolygon(Level* _level, const std::vector<FVector2>& _points, const Color& _color = Color::Magenta());

		// Point
		static void DrawDebugPoint(Actor* _context, const FVector2& _position, const Float& _thickness = 5.0f, const Color& _color = Color::Magenta());
		static void DrawDebugPoint(Level* _level, const FVector2& _position, const Float& _thickness = 5.0f, const Color& _color = Color::Magenta());
	
	
		// Line
		static void DrawDebugLine(Actor* _context, const FVector2& _start, const FVector2& _direction, const Float& _length = 75.0f, const Float& _thickness = 2.5f, const Color& _color = Color::Magenta());
		static void DrawDebugLine(Level* _level, const FVector2& _start, const FVector2& _direction, const Float& _length = 75.0f, const Float& _thickness = 2.5f, const Color& _color = Color::Magenta());
	};

}

