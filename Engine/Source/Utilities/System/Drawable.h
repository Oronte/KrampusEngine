#pragma once

namespace Krampus
{
	class Window;

	enum class ZOrder
	{
		Background = 0,
		World = 25,
		Actors = 50,
		Projectiles = 75,
		Effects = 100,
		Foreground = 125,
		Environment = 150,
		Particles = 175,
		Widgets = 200,
		Debug = 225
	};

	/// <summary>
	/// Interface for drawable objects.
	/// </summary>
	class IDrawable
	{
		uint8_t zOrder = 0;
	public:
		INLINE uint8_t GetZOrder() const
		{
			return zOrder;
		}
		INLINE virtual void SetZOrder(const uint8_t& _zOrder)
		{
			zOrder = _zOrder;
		}
		INLINE virtual void SetZOrder(const ZOrder& _zOrder)
		{
			zOrder = CAST(uint8_t, _zOrder);
		}
		virtual void Draw(Window& _window) = 0;
	};

}

/////////////////////////////////////////////////
// 
// class DrawableClass : public IDrawable 
// {
//     virtual void Draw(Window& _window) override
//     {
//         _window.Draw(objectToDraw);
//     }
// }
// 
/////////////////////////////////////////////////