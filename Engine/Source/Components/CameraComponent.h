#pragma once
#include "Component.h"

namespace Krampus
{

	class CameraComponent : public Component
	{
		std::unique_ptr<sf::View>	view;

		Event<FVector2>::ListenerHandle onOwnerMoveHandle;
		Event<Angle>::ListenerHandle onOwnerRotateHandle;

	public:
		Bool						attachedToOwner		= false;
		Bool						freezePostition		= false;
		Bool						freezeRotation		= false;


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
		inline void Zoom(const Float& _factor)
		{
			view->zoom(_factor);
		}

		CameraComponent(Actor* _owner);
		CameraComponent(Actor* _owner, const FVector2& _center, const FVector2& _size);
		CameraComponent(Actor* _owner, const FVector2& _size);
		CameraComponent(Actor* _owner, const FRect& _rect);

		void SetCurrent();

	private:
		virtual void BeginDestroy() override;

		void InitEvent();

	public:
		virtual std::string ToString() const override;
	};

}

