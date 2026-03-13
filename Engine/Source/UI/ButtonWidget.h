#pragma once
#include "ImageWidget.h"

namespace Krampus
{

	class ButtonWidget : public ImageWidget
	{
	protected:
		Bool							isPressed			= false;
		Bool							isHovered			= false;

		Event<IVector2>::ListenerHandle onMouseMovedHandle;
		Event<>::ListenerHandle			onClickHandle;
		Event<>::ListenerHandle			onReleaseHandle;

	public:
		Event<>							onHover;
		Event<>							onUnhover;
		Event<>							onPress;
		Event<>							onPerform;
		Event<>							onRelease;

		ButtonWidget(Level* _level, const CircleShapeData& _data);
		ButtonWidget(Level* _level, const RectangleShapeData& _data);
		
	protected:
		virtual void Tick(const Float& _deltaTime) override;

	private:
		void Init();

		void OnMouseMoved(const IVector2& _mousePos);
		void OnClick();
		void OnRelease();

	public:
		virtual std::string ToString() const override;
	};

}

