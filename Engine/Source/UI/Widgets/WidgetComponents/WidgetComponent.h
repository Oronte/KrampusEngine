#pragma once
#include "Core/Engine.h"
#include "Core/GameObject.h"
#include "Utilities/System/Timer.h"
#include "Utilities/System/Transformable.h"

namespace Krampus
{
    /// <summary>
    /// Base class for modular components attached to 2D HUD widgets.
    /// </summary>
    class WidgetComponent : public IGameObject, public ITransformable
    {
    protected:
        class Widget*	owner;

    public:
        /// <summary>Gets the owning Widget.</summary>
        /// <returns>Pointer to owning Widget.</returns>
        inline Widget* GetOwner() const
        {
            return owner;
        }

        /// <summary>Gets the Level owning this component's widget.</summary>
        /// <returns>Pointer to Level.</returns>
        class Level* GetLevel() const;

        /// <summary>Gets the Engine world instance.</summary>
        /// <returns>Pointer to Engine.</returns>
        Engine* GetWorld() const;

        /// <summary>Gets the frame delta time in seconds.</summary>
        /// <returns>Delta time as Float.</returns>
        Float GetDeltaTime() const;

    protected:
        /// <summary>Constructs a WidgetComponent attached to a Widget.</summary>
        /// <param name="_owner">Pointer to owning Widget.</param>
        WidgetComponent(Widget* _owner);

        /// <summary>Creates a timer managed within the widget level.</summary>
        /// <param name="_callback">Timeout callback function.</param>
        /// <param name="_duration">Timer duration in seconds.</param>
        /// <param name="_isLoop">Whether timer repeats.</param>
        /// <param name="_startRunning">Whether timer starts immediately.</param>
        /// <returns>Pointer to created Timer.</returns>
        Timer* CreateTimer(const std::function<void()>& _callback, const Float& _duration, const Bool& _isLoop = false, const Bool& _startRunning = true);

    public:
        /// <summary>Virtual destructor.</summary>
        virtual ~WidgetComponent() = default;

    public:
        /// <summary>Lifecycle callback on component creation.</summary>
        virtual void Construct() override;
        /// <summary>Lifecycle callback on component deconstruction.</summary>
        virtual void Deconstruct() override;
        /// <summary>Lifecycle callback when gameplay starts.</summary>
        virtual void BeginPlay() override {};
        /// <summary>Updates component state each frame.</summary>
        /// <param name="_deltaTime">Elapsed time in seconds.</param>
        virtual void Tick(const Float& _deltaTime) override {};
        /// <summary>Lifecycle callback before component is destroyed.</summary>
        virtual void BeginDestroy() override {};

        /// <summary>Returns a string representation of the component.</summary>
        /// <returns>String representation.</returns>
        virtual std::string ToString() const override;

        /// <summary>Sets relative position coordinates.</summary>
        /// <param name="_newPosition">Position vector.</param>
        void SetPosition(const FVector2& _newPosition) override;
        /// <summary>Gets relative position coordinates.</summary>
        /// <returns>Position vector.</returns>
        FVector2 GetPosition() const override;
        /// <summary>Sets relative rotation angle.</summary>
        /// <param name="_newRotation">Angle.</param>
        void SetRotation(const Angle& _newRotation) override;
        /// <summary>Gets relative rotation angle.</summary>
        /// <returns>Current Angle.</returns>
        Angle GetRotation() const override;
        /// <summary>Sets relative scale factors.</summary>
        /// <param name="_newScale">Scale multipliers.</param>
        void SetScale(const FVector2& _newScale) override;
        /// <summary>Gets relative scale factors.</summary>
        /// <returns>Scale multipliers.</returns>
        FVector2 GetScale() const override;
    };
}
