#pragma once
#include "BasicImage.h"

namespace Krampus
{
	class StainComponent;

	/// <summary>A shape drawn in the world at its actor's transform, with the marks of a StainComponent over it.</summary>
	class SpriteComponent : public BasicImage<Component>
	{
		StainComponent* stainLayer = nullptr;

	public:
		SpriteComponent(Actor* _owner, const CircleShapeData& _data)	: BasicImage(_owner, _data) { name = NAME_OF(SpriteComponent); }
		SpriteComponent(Actor* _owner, const RectangleShapeData& _data) : BasicImage(_owner, _data) { name = NAME_OF(SpriteComponent); }

		void SetStainLayer(StainComponent* _stains)				{ stainLayer = _stains; }
		[[nodiscard]] StainComponent* GetStainLayer() const		{ return stainLayer; }

		void Draw(Window& _window) override;
	};
}
