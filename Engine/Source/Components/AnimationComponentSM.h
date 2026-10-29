#pragma once
#include "Component.h"
#include "Graphics/Ressources/Animation.h"

namespace Krampus
{

	class AnimationComponentSM;

	/// <summary>
	/// Represents an animation state in an animation state machine with conditional transitions.
	/// </summary>
	struct AnimationSM : public Animation
	{
	private:
		AnimationComponentSM*							component	= nullptr;
		std::map<AnimationSM*, std::function<Bool()>>	links;

	public:
		/// <summary>
		/// Constructs an animation state machine node.
		/// </summary>
		/// <param name="_engine">Pointer to the Engine.</param>
		/// <param name="_component">Pointer to the owning AnimationComponentSM.</param>
		/// <param name="_name">Animation name identifier.</param>
		/// <param name="_shape">Pointer to the target shape being animated.</param>
		/// <param name="_data">Animation frames and parameters.</param>
		AnimationSM(Engine* _engine, AnimationComponentSM* _component, const String& _name, ShapeObject* _shape, const AnimationData& _data);
	
		/// <summary>
		/// Creates a conditional transition link to another animation state.
		/// </summary>
		/// <param name="_animation">Target animation state to transition to.</param>
		/// <param name="_check">Condition callback predicate returning true to trigger transition.</param>
		/// <returns>True if transition was created successfully.</returns>
		Bool CreateLink(AnimationSM* _animation, std::function<Bool()> _check);

		/// <summary>
		/// Evaluates condition predicates of all transitions and returns the next state if one passes.
		/// </summary>
		/// <returns>Pointer to the next AnimationSM state, or nullptr if no condition succeeded.</returns>
		AnimationSM* GetNextAnimation();
	};

	/// <summary>
	/// Component that manages animations via a state machine with automated conditional transitions.
	/// </summary>
	class AnimationComponentSM : public Component
	{
		AnimationSM*												current		= nullptr;
		std::unordered_map<String, std::unique_ptr<AnimationSM>>	animations;
		std::vector<String>	appliedClips;	// names of the animations made from clips, so they can be replaced
		bool				clipStarted	= false;

	public:
		/// <summary>Animations described by plain values (Inspector, Animation Editor, scenes). Call ApplyClips after changing them.</summary>
		std::vector<AnimationClip>	clips;

		/// <summary>Name of the clip that is the current animation when the level starts (empty or unknown: the first clip).</summary>
		String				startAnimation;

		/// <summary>Starts the clip animation when the level starts.</summary>
		Bool				playOnBeginPlay	= false;

		/// <summary>(Re)creates the animations described by clips on the sprite of the owner and makes startAnimation the current one.</summary>
		void ApplyClips();


	public:
		/// <summary>
		/// Gets the currently active animation state.
		/// </summary>
		/// <returns>Pointer to the active AnimationSM, or nullptr.</returns>
		inline AnimationSM* GetCurrentAnimation() const
		{
			return current;
		}

		/// <summary>
		/// Retrieves an animation state by name.
		/// </summary>
		/// <param name="_animationName">Animation name identifier.</param>
		/// <returns>Pointer to AnimationSM, or nullptr if not found.</returns>
		inline AnimationSM* GetAnimation(const String& _animationName) const
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
		/// Starts playback of the active animation.
		/// </summary>
		inline void StartAnimation()
		{
			if (!current) return;
			current->Start();
		}

		/// <summary>
		/// Stops playback of the active animation.
		/// </summary>
		inline void StopAnimation()
		{
			if (!current) return;
			current->Stop();
		}

		/// <summary>
		/// Resumes playback of the active animation.
		/// </summary>
		inline void ResumeAnimation()
		{
			if (!current) return;
			current->Resume();
		}

		/// <summary>
		/// Pauses playback of the active animation.
		/// </summary>
		inline void PauseAnimation()
		{
			if (!current) return;
			current->Pause();
		}

		/// <summary>
		/// Constructs an AnimationComponentSM attached to the specified actor.
		/// Note: The owner requires an existing SpriteComponent.
		/// </summary>
		/// <param name="_owner">Pointer to owning Actor.</param>
		AnimationComponentSM(Actor* _owner);

		/// <summary>
		/// Adds an animation state to the state machine without initial links.
		/// </summary>
		/// <param name="_name">Name identifier for the state.</param>
		/// <param name="_data">Animation frames and settings.</param>
		/// <returns>Pointer to the created AnimationSM.</returns>
		AnimationSM* AddAnimation(const String& _name, const AnimationData& _data);

		/// <summary>
		/// Adds an animation state to the state machine with a list of conditional transition links.
		/// </summary>
		/// <param name="_name">Name identifier for the state.</param>
		/// <param name="_data">Animation frames and settings.</param>
		/// <param name="_links">Vector of target animation states and condition callbacks.</param>
		/// <returns>Pointer to the created AnimationSM.</returns>
		AnimationSM* AddAnimation(const String& _name, const AnimationData& _data, std::vector<std::pair<AnimationSM*, std::function<Bool()>>> _links);

		/// <summary>
		/// Updates the state machine, evaluates transition links, and updates frames.
		/// </summary>
		/// <param name="_deltaTime">Elapsed time in seconds.</param>
		virtual void Tick(const Float& _deltaTime) override;

		/// <summary>
		/// Returns a string representation of the component.
		/// </summary>
		/// <returns>String representation.</returns>
		virtual std::string ToString() const override;

		friend struct AnimationSM;
	};

}
