#pragma once
#include "Core/CoreMinimal.h"

namespace Krampus
{

    class Layer {
    public:
        virtual ~Layer() = default;
        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(Float _dt) {}
        virtual void OnRender(Window& _w) {}
        virtual Bool OnEvent(const sf::Event& _e) { return false; } // true = consume
    };

}