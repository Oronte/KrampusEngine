#pragma once
#include "Core/Layer.h"

namespace Krampus
{
	/// <summary>
	/// Ordered list of layers. Layers sit below overlays, so an overlay is always drawn over and receives events before any layer.
	/// </summary>
	class LayerStack
	{
		std::vector<std::unique_ptr<Layer>>		layers;
		size_t									overlayStart = 0;

	public:
		~LayerStack()
		{
			Clear();
		}

		/// <summary>
		/// Pushes a layer on top of the other layers but below all overlays.
		/// </summary>
		/// <returns>Pointer to the new layer.</returns>
		template <typename Type, typename ...Args>
		inline Type* PushLayer(Args&&... _args)
		{
			Type* _layer = new Type(std::forward<Args>(_args)...);
			layers.emplace(layers.begin() + overlayStart, _layer);
			++overlayStart;
			_layer->OnAttach();
			return _layer;
		}

		/// <summary>
		/// Pushes an overlay on top of everything.
		/// </summary>
		/// <returns>Pointer to the new overlay.</returns>
		template <typename Type, typename ...Args>
		inline Type* PushOverlay(Args&&... _args)
		{
			Type* _layer = new Type(std::forward<Args>(_args)...);
			layers.emplace_back(_layer);
			_layer->OnAttach();
			return _layer;
		}

		/// <summary>
		/// Removes a layer or overlay from the stack. Does nothing if it is not in the stack.
		/// </summary>
		inline void Pop(Layer* _layer)
		{
			auto _it = std::find_if(layers.begin(), layers.end(), [&](const auto& _other) { return _other.get() == _layer; });
			if (_it == layers.end()) return;

			if (static_cast<size_t>(_it - layers.begin()) < overlayStart)
				--overlayStart;

			(*_it)->OnDetach();
			layers.erase(_it);
		}

		/// <summary>
		/// Removes every layer, top first.
		/// </summary>
		inline void Clear()
		{
			while (!layers.empty())
			{
				layers.back()->OnDetach();
				layers.pop_back();
			}
			overlayStart = 0;
		}

		inline void Update(const Float& _deltaTime)
		{
			// Layers below the topmost pausing layer are skipped.
			size_t _first = 0;
			for (size_t _i = layers.size(); _i > 0; --_i)
				if (layers[_i - 1]->PausesLayersBelow())
				{
					_first = _i - 1;
					break;
				}

			for (size_t _i = _first; _i < layers.size(); ++_i)
				layers[_i]->OnUpdate(_deltaTime);

			for (size_t _i = layers.size(); _i > 0; --_i)
				if (layers[_i - 1]->IsClosed())
					Pop(layers[_i - 1].get());
		}

		/// <summary>
		/// True if any layer asks the InputManager to be cut off.
		/// </summary>
		inline Bool BlocksInput() const
		{
			for (const auto& _layer : layers)
				if (_layer->BlocksInput())
					return true;
			return false;
		}

		inline void Render(Window& _window)
		{
			for (size_t _i = 0; _i < layers.size(); ++_i)
				layers[_i]->OnRender(_window);
		}

		/// <summary>
		/// Sends the event from the top layer down until one consumes it.
		/// </summary>
		/// <returns>True if a layer consumed the event.</returns>
		inline Bool DispatchEvent(const sf::Event& _event)
		{
			for (size_t _i = layers.size(); _i > 0; --_i)
				if (layers[_i - 1]->OnEvent(_event))
					return true;
			return false;
		}
	};
}