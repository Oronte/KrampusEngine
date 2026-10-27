#pragma once
#include "DiegeticImageWidget.h"

namespace Krampus
{

	/// <summary>
	/// In-world diegetic button actor supporting mouse hover, press, hold, and release interactions.
	/// </summary>
	class DiegeticButtonWidget : public DiegeticImageWidget
	{
	protected:
		Bool							isPressed			= false;
		Bool							isHovered			= false;

		Event<IVector2>::ListenerHandle onMouseMovedHandle;
		Event<>::ListenerHandle			onClickHandle;
		Event<>::ListenerHandle			onReleaseHandle;

	public:
		/// <summary>Broadcast when cursor enters button bounds.</summary>
		Event<>							onHover;
		/// <summary>Broadcast when cursor leaves button bounds.</summary>
		Event<>							onUnhover;
		/// <summary>Broadcast when button is first clicked down.</summary>
		Event<>							onPress;
		/// <summary>Broadcast every frame while button is held down.</summary>
		Event<>							onPerform;
		/// <summary>Broadcast when button click is released.</summary>
		Event<>							onRelease;

		/// <summary>Constructs a circular diegetic button.</summary>
		/// <param name="_level">Owning Level.</param>
		/// <param name="_data">Circle parameters.</param>
		DiegeticButtonWidget(Level* _level, const CircleShapeData& _data);

		/// <summary>Constructs a rectangular diegetic button.</summary>
		/// <param name="_level">Owning Level.</param>
		/// <param name="_data">Rectangle parameters.</param>
		DiegeticButtonWidget(Level* _level, const RectangleShapeData& _data);
		
	protected:
		/// <summary>Updates button interaction state each frame.</summary>
		/// <param name="_deltaTime">Elapsed time in seconds.</param>
		virtual void Tick(const Float& _deltaTime) override;

	private:
		void Init();

		void OnMouseMoved(const IVector2& _mousePos);
		void OnClick();
		void OnRelease();

	public:
		/// <summary>Returns a string representation of the diegetic button.</summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;
	};

}
