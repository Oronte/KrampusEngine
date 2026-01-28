#pragma once
#include "Component.h"

namespace Krampus
{

	class CameraComponent : public Component
	{
		std::unique_ptr<sf::View> view;

	public:
		bool attachedToOwner = true;


		inline sf::View* GetView() const
		{
			return view.get();
		}
		inline FRect GetViewport() const
		{
			return view->getViewport();
		}
		inline void SetViewport(const FRect& _rect)
		{
			view->setViewport(_rect);
		}
		inline void SetCenter(const FVector2& _center)
		{
			view->setCenter(_center);
		}
		inline void SetRotation(const Angle& _angle)
		{
			view->setRotation(_angle);
		}
		inline void SetSize(const FVector2& _size)
		{
			view->setSize(_size);
		}
		inline void SetScissor(const FRect& _rect)
		{
			view->setScissor(_rect);
		}
		inline void Move(const FVector2& _offset)
		{
			view->move(_offset);
		}
		inline void Rotate(const Angle& _angle)
		{
			view->rotate(_angle);
		}
		inline void Zoom(const float _factor)
		{
			view->zoom(_factor);
		}

		CameraComponent(Actor* _owner);
		CameraComponent(Actor* _owner, const FVector2& _center, const FVector2& _size);
		CameraComponent(Actor* _owner, const FVector2& _size);
		CameraComponent(Actor* _owner, const FRect& _rect);

	private:
		virtual void Tick(const float& _deltaTime) override;
	};

}

