#pragma once
#include "WidgetComponent.h"
#include "Graphics/Ressources/Animation.h"

namespace Krampus
{
    /// <summary>
    /// Component that manages and plays sprite animations within a HUD widget.
    /// </summary>
    class AnimationWidgetComponent : public WidgetComponent
    {
        Animation*														current		= nullptr;
        std::unordered_map<String, std::unique_ptr<Animation>>			animations;
		String				appliedClip;	// name of the animation made from clip, so it can be replaced
		bool				clipStarted	= false;

	public:
		/// <summary>Animation described by plain values (Inspector, scenes). Call ApplyClip after changing it.</summary>
		AnimationClip		clip;

		/// <summary>Starts the clip animation when the level starts.</summary>
		Bool				playOnBeginPlay	= false;

		/// <summary>(Re)creates the animation described by clip on the sprite of the owner and makes it the current one.</summary>
		void ApplyClip();

		virtual void Tick(const Float& _deltaTime) override;


    public:
        /// <summary>Sets the active animation by name.</summary>
        /// <param name="_name">Animation name identifier.</param>
        inline void SetCurrentAnimation(const String& _name)
        {
            if (!animations.contains(_name)) return;

            if (current) current->Stop();
            current = animations[_name].get();
        }

        /// <summary>Gets a pointer to the active animation.</summary>
        /// <returns>Pointer to active Animation.</returns>
        inline Animation* GetCurrentAnimation() const
        {
            return current;
        }

        /// <summary>Gets an animation by name.</summary>
        /// <param name="_animationName">Animation name identifier.</param>
        /// <returns>Pointer to Animation, or nullptr if not found.</returns>
        inline Animation* GetAnimation(const String& _animationName) const
        {
            auto _iterator = animations.find(_animationName);
            if (_iterator == animations.end())
            {
                LOG_ERROR("There is no animation with the name \"" + _animationName + "\"");
                return nullptr;
            }
            return _iterator->second.get();
        }

        /// <summary>Starts animation playback.</summary>
        inline void StartAnimation()
        {
            if (!current) return;
            current->Start();
        }

        /// <summary>Stops animation playback.</summary>
        inline void StopAnimation()
        {
            if (!current) return;
            current->Stop();
        }

        /// <summary>Resumes paused animation playback.</summary>
        inline void ResumeAnimation()
        {
            if (!current) return;
            current->Resume();
        }

        /// <summary>Pauses animation playback.</summary>
        inline void PauseAnimation()
        {
            if (!current) return;
            current->Pause();
        }

        /// <summary>
        /// Constructs an AnimationWidgetComponent attached to a widget.
        /// Note: The owning widget requires an existing ImageWidgetComponent.
        /// </summary>
        /// <param name="_owner">Pointer to owning Widget.</param>
        AnimationWidgetComponent(Widget* _owner);

        /// <summary>Adds a new animation with specified frame settings.</summary>
        /// <param name="_name">Animation name identifier.</param>
        /// <param name="_data">Animation frame configuration.</param>
        virtual void AddAnimation(const String& _name, const AnimationData& _data);

        /// <summary>Lifecycle callback on component deconstruction.</summary>
        virtual void Deconstruct() override;

        /// <summary>Returns a string representation of the component.</summary>
        /// <returns>String representation.</returns>
        virtual std::string ToString() const override;
    };
}
