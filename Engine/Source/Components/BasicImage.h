#pragma once
#include "DrawnComponent.h"
#include "Graphics/Shapes/ShapeObject.h"
#include "Graphics/Shaders/ShaderMaterial.h"
#include "Utilities/System/Debuggable.h"

namespace Krampus
{

	/// <summary>
	/// A shape (circle, rectangle or polygon, textured or not) that follows its owner's transform.
	/// Shared by SpriteComponent (world) and ImageWidgetComponent (HUD).
	/// </summary>
	template <typename Base>
	class BasicImage : public DrawnComponent<Base>, public IDebuggable, public IShaded
	{
		Event<FVector2>::ListenerHandle onOwnerMoveHandle;
		Event<Angle>::ListenerHandle	onOwnerRotateHandle;
		Event<FVector2>::ListenerHandle onOwnerScaleHandle;

	protected:
		std::unique_ptr<ShapeObject> shape;

		BasicImage(typename Base::OwnerType* _owner, const CircleShapeData& _data);
		BasicImage(typename Base::OwnerType* _owner, const RectangleShapeData& _data);

	public:
		[[nodiscard]] ShapeObject* GetShapeObject()				{ return shape.get(); }
		[[nodiscard]] const ShapeObject* GetShapeObject() const	{ return shape.get(); }
		[[nodiscard]] ShapeSizeData& GetShapeSizeData()			{ return shape->GetSizeData(); }
		[[nodiscard]] ShapeType GetShapeType() const			{ return shape->GetShapeType(); }

		void SetSize(const FVector2& _size)		{ shape->SetSize(_size); }
		[[nodiscard]] FVector2 GetSize() const	{ return shape->GetSize(); }
		void SetRadius(const Float& _radius)	{ shape->SetRadius(_radius); }
		[[nodiscard]] Float GetRadius() const	{ return shape->GetRadius(); }
		void SetPointCount(const size_t& _pointCount)	{ shape->SetPointCount(_pointCount); }
		[[nodiscard]] size_t GetPointCount() const		{ return shape->GetPointCount(); }

		/// <summary>Sets the picture (path relative to Content/Textures, no extension). An empty rect = the whole picture.</summary>
		void SetTexture(const std::string& _path, TextureExtensionType _textureType = TextureExtensionType::PNG, const IRect& _rect = IRect(), const Bool& _isRepeated = false, const Bool& _smooth = true)
		{
			shape->SetTexture(_path, _textureType, _rect, _isRepeated, _smooth);
		}

		void Construct() override;
		void Draw(Window& _window) override;
		std::string ToString() const override;

	private:
		void DrawDebug() override;
		void FollowOwner(typename Base::OwnerType* _owner);
	};

	extern template class BasicImage<Component>;
	extern template class BasicImage<WidgetComponent>;

}
