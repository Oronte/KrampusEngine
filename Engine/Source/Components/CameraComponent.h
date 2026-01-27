#pragma once
#include "Component.h"

namespace Krampus
{

	class CameraComponent : public Component
	{
		std::unique_ptr<sf::View> view;

	public:
		bool attachedToOwner = true;


		inlin sf::View* GetView() const
		{
			return view.get();
		}
		inlin FRect GetViewport() const
		{
			return view->getViewport();
		}
		inlin void SetViewport(const FRect& _rect)
		{
			view->setViewport(_rect);
		}
		inlin void SetCenter(const FVector2& _center)
		{
			view->setCenter(_center);
		}
		inlin void SetRotation(const Angle& _angle)
		{
			view->setRotation(_angle);
		}
		inlin void SetSize(const FVector2& _size)
		{
			view->setSize(_size);
		}
		inlin void SetScissor(const FRect& _rect)
		{
			view->setScissor(_rect);
		}
		inlin void Move(const FVector2& _offset)
		{
			view->move(_offset);
		}
		inlin void Rotate(const Angle& _angle)
		{
			view->rotate(_angle);
		}
		inlin void Zoom(const float _factor)
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

