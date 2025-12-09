#pragma once
#include "Component.h"

namespace Krampus
{

	class CameraComponent : public Component
	{
		std::unique_ptr<sf::View> view;
		//bool attachedToOwner = true; // TODO Attach camera to owner

	public:
		FORCEINLINE sf::View* GetView() const
		{
			return view.get();
		}
		FORCEINLINE FRect GetViewport() const
		{
			return view->getViewport();
		}
		FORCEINLINE void SetViewport(const FRect& _rect)
		{
			view->setViewport(_rect);
		}
		FORCEINLINE void SetCenter(const FVector2& _center)
		{
			view->setCenter(_center);
		}
		FORCEINLINE void SetRotation(const Angle& _angle)
		{
			view->setRotation(_angle);
		}
		FORCEINLINE void SetSize(const FVector2& _size)
		{
			view->setSize(_size);
		}
		FORCEINLINE void SetScissor(const FRect& _rect)
		{
			view->setScissor(_rect);
		}
		FORCEINLINE void Move(const FVector2& _offset)
		{
			view->move(_offset);
		}
		FORCEINLINE void Rotate(const Angle& _angle)
		{
			view->rotate(_angle);
		}
		FORCEINLINE void Zoom(const float _factor)
		{
			view->zoom(_factor);
		}

		CameraComponent(Actor* _owner);
		CameraComponent(Actor* _owner, const FVector2& _center, const FVector2& _size);
		CameraComponent(Actor* _owner, const FVector2& _size);
		CameraComponent(Actor* _owner, const FRect& _rect);
	};

}

