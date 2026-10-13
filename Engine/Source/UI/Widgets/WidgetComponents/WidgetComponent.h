#pragma once
#include "Core/Engine.h"
#include "Core/GameObject.h"
#include "Utilities/System/Timer.h"
#include "Utilities/System/Transformable.h"

namespace Krampus
{
    class WidgetComponent : public IGameObject, public ITransformable
    {
    protected:
        class Widget*	owner;

    public:
        inline Widget* GetOwner() const
        {
            return owner;
        }
        class Level* GetLevel() const;
        Engine* GetWorld() const;
        Float GetDeltaTime() const;

    protected:
        WidgetComponent(Widget* _owner);

        Timer* CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);

    public:
        virtual ~WidgetComponent() = default;

    public:
        virtual void Construct() override;
        virtual void Deconstruct() override;
        virtual void BeginPlay() override {};
        virtual void Tick(const Float& _deltaTime) override {};
        virtual void BeginDestroy() override {};

        virtual std::string ToString() const override;

        void SetPosition(const FVector2& _newPosition) override;
        FVector2 GetPosition() const override;
        void SetRotation(const Angle& _newRotation) override;
        Angle GetRotation() const override;
        void SetScale(const FVector2& _newScale) override;
        FVector2 GetScale() const override;
    };
}
