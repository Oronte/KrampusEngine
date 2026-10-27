#pragma once
#include "Graphics/Window.h"

namespace Krampus
{

	/// <summary>
	/// Base class for a slice of the application (level, UI, pause menu, debug...) updated, drawn and fed events in stack order.
	/// </summary>
	class Layer
	{
	public:
		virtual ~Layer() = default;

		/// <summary>Called when the layer is pushed on the stack.</summary>
		virtual void OnAttach() { }

		/// <summary>Called when the layer is popped from the stack.</summary>
		virtual void OnDetach() { }

		/// <summary>Called once per frame, bottom layer first.</summary>
		virtual void OnUpdate(const Float& _deltaTime) { }

		/// <summary>Called once per frame after all updates, bottom layer first (top layer is drawn last, so on top).</summary>
		virtual void OnRender(Window& _window) { }

		/// <summary>
		/// Called for each window event, top layer first.
		/// </summary>
		/// <returns>True to consume the event so lower layers and the InputManager never see it.</returns>
		virtual Bool OnEvent(const sf::Event& _event) { return false; }

		/// <summary>When true, layers below this one are still drawn but no longer updated.</summary>
		virtual Bool PausesLayersBelow() const { return false; }

		/// <summary>When true, the InputManager receives nothing (keys, mouse, pads).</summary>
		virtual Bool BlocksInput() const { return false; }

		/// <summary>Asks the stack to remove this layer at the end of the frame. Safe to call from any callback.</summary>
		void Close() { closed = true; }

		Bool IsClosed() const { return closed; }

	private:
		Bool closed = false;
	};
}
