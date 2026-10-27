#pragma once
#include "GameFramework/Layer.h"

namespace Krampus
{
	class LayerStack
	{
		std::vector<std::unique_ptr<Layer>> layers;
		size_t insertIndex = 0;
	
	public:
		LayerStack() = default;
		~LayerStack();

		Layer* PushLayer(std::unique_ptr<Layer> _layer);
		Layer* PushOverlay(std::unique_ptr<Layer> _overlay);
		void PopLayer(Layer* _layer);
		void PopOverlay(Layer* _overlay);
	};
}