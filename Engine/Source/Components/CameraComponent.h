#pragma once
#include "Component.h"

namespace Krampus
{

	/// <summary>
	/// Component that provides a 2D camera view into the scene, managing viewport, zoom, and orientation.
	/// </summary>
	class CameraComponent : public Component
	{
		std::unique_ptr<sf::View>	view;

		Event<FVector2>::ListenerHandle onOwnerMoveHandle;
		Event<Angle>::ListenerHandle onOwnerRotateHandle;

	public:
		/// <summary>
		/// Whether the camera viewpoint automatically follows the owner actor's position and rotation.
		/// </summary>
		Bool						attachedToOwner		= false;

		/// <summary>
		/// Prevents the camera center from moving if true.
		/// </summary>
		Bool						freezePostition		= false;

		/// <summary>
		/// Prevents the camera from rotating if true.
		/// </summary>
		Bool						freezeRotation		= false;

		/// <summary>
		/// Gets a pointer to the underlying SFML view.
		/// </summary>
		/// <returns>Pointer to sf::View.</returns>
		inline sf::View* GetView() const
		{
			return view.get();
		}

		/// <summary>
		/// Gets the normalized viewport rectangle in the window [0, 1].
		/// </summary>
		/// <returns>FRect representing viewport bounds.</returns>
		inline FRect GetViewport() const
		{
			return view->getViewport();
		}

		/// <summary>
		/// Sets the normalized viewport rectangle in the window [0, 1].
		/// </summary>
		/// <param name="_rect">The viewport rectangle.</param>
		inline void SetViewport(const FRect& _rect)
		{
			view->setViewport(_rect);
		}

		/// <summary>
		/// Sets the center coordinates of the camera view in world units.
		/// </summary>
		/// <param name="_center">World position of the view center.</param>
		inline void SetCenter(const FVector2& _center)
		{
			view->setCenter(_center);
		}

		/// <summary>
		/// Sets the rotation angle of the camera view.
		/// </summary>
		/// <param name="_angle">Rotation angle.</param>
		inline void SetRotation(const Angle& _angle)
		{
			view->setRotation(_angle);
		}

		/// <summary>
		/// Sets the visible dimensions (width and height) of the camera view.
		/// </summary>
		/// <param name="_size">View dimensions vector.</param>
		inline void SetSize(const FVector2& _size)
		{
			view->setSize(_size);
		}

		/// <summary>
		/// Sets the scissor rectangle for rendering through this camera view.
		/// </summary>
		/// <param name="_rect">Scissor bounds rectangle.</param>
		inline void SetScissor(const FRect& _rect)
		{
			view->setScissor(_rect);
		}

		/// <summary>
		/// Translates the camera view by an offset.
		/// </summary>
		/// <param name="_offset">Offset vector to move the center.</param>
		inline void Move(const FVector2& _offset)
		{
			view->move(_offset);
		}

		/// <summary>
		/// Rotates the camera view by an angle delta.
		/// </summary>
		/// <param name="_angle">Angle offset.</param>
		inline void Rotate(const Angle& _angle)
		{
			view->rotate(_angle);
		}

		/// <summary>
		/// Zooms the camera view by a scale factor. Values &gt; 1 zoom out, &lt; 1 zoom in.
		/// </summary>
		/// <param name="_factor">Zoom multiplier.</param>
		inline void Zoom(const Float& _factor)
		{
			view->zoom(_factor);
		}

		/// <summary>
		/// Constructs a CameraComponent using the window's default view dimensions.
		/// </summary>
		/// <param name="_owner">Pointer to the owning Actor.</param>
		CameraComponent(Actor* _owner);

		/// <summary>
		/// Constructs a CameraComponent with a specified center position and size.
		/// </summary>
		/// <param name="_owner">Pointer to the owning Actor.</param>
		/// <param name="_center">Center position in world coordinates.</param>
		/// <param name="_size">View dimensions.</param>
		CameraComponent(Actor* _owner, const FVector2& _center, const FVector2& _size);

		/// <summary>
		/// Constructs a CameraComponent with a specified size centered at origin.
		/// </summary>
		/// <param name="_owner">Pointer to the owning Actor.</param>
		/// <param name="_size">View dimensions.</param>
		CameraComponent(Actor* _owner, const FVector2& _size);

		/// <summary>
		/// Constructs a CameraComponent initialized from a rectangle.
		/// </summary>
		/// <param name="_owner">Pointer to the owning Actor.</param>
		/// <param name="_rect">Rectangle defining the view bounds.</param>
		CameraComponent(Actor* _owner, const FRect& _rect);

		/// <summary>
		/// Activates this camera as the current active rendering view in the level.
		/// </summary>
		void SetCurrent();

	private:
		virtual void BeginDestroy() override;

		void InitEvent();

	public:
		/// <summary>
		/// Returns a string representation of the component.
		/// </summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;
	};

}
