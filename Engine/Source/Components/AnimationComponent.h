#pragma once
#include "Component.h"
#include "Graphics/Ressources/Animation.h"

namespace Krampus
{

	/// <summary>
	/// Component that manages and plays 2D frame-by-frame sprite animations for an actor.
	/// </summary>
	class AnimationComponent : public Component
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
		/// <summary>
		/// Sets the currently active animation by name.
		/// </summary>
		/// <param name="_name">The name identifier of the animation.</param>
		inline void SetCurrentAnimation(const String& _name)
		{
			if (!animations.contains(_name)) return;

			if (current) current->Stop();
			current = animations[_name].get();
		}

		/// <summary>
		/// Gets a pointer to the currently active animation.
		/// </summary>
		/// <returns>Pointer to current Animation, or nullptr if none is set.</returns>
		inline Animation* GetCurrentAnimation() const
		{
			return current;
		}

		/// <summary>
		/// Retrieves an animation by its registered name.
		/// </summary>
		/// <param name="_animationName">The name identifier of the animation.</param>
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

		/// <summary>
		/// Starts playback of the current animation.
		/// </summary>
		inline void StartAnimation()
		{
			if (!current) return;
			current->Start();
		}

		/// <summary>
		/// Stops playback of the current animation and resets to the first frame.
		/// </summary>
		inline void StopAnimation()
		{
			if (!current) return;
			current->Stop();
		}

		/// <summary>
		/// Resumes playback of the current animation from a paused state.
		/// </summary>
		inline void ResumeAnimation()
		{
			if (!current) return;
			current->Resume();
		}

		/// <summary>
		/// Pauses playback of the current animation at the current frame.
		/// </summary>
		inline void PauseAnimation()
		{
			if (!current) return;
			current->Pause();
		}

		/// <summary>
		/// Constructs an AnimationComponent attached to the specified actor.
		/// Note: The owning actor requires an existing SpriteComponent.
		/// </summary>
		/// <param name="_owner">Pointer to the owning Actor.</param>
		AnimationComponent(Actor* _owner);

		/// <summary>
		/// Adds a new animation with the specified configuration data.
		/// </summary>
		/// <param name="_name">Name identifier for the animation.</param>
		/// <param name="_data">Animation frames and playback settings.</param>
		virtual void AddAnimation(const String& _name, const AnimationData& _data);

		/// <summary>
		/// Lifecycle callback invoked when the component is deconstructed.
		/// </summary>
		virtual void Deconstruct() override;

		/// <summary>
		/// Returns a string representation of the component.
		/// </summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;
	};

}
