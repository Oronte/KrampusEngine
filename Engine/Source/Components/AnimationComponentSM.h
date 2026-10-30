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
		/// Evaluates condition predicates of all transitions and returns the next state if one passes (one step per frame).
		/// </summary>
		/// <returns>Pointer to the next AnimationSM state, or nullptr if no condition succeeded.</returns>
		AnimationSM* GetNextAnimation();

		/// <summary>Removes the code links that go to _animation (it is being deleted).</summary>
		void RemoveLinksTo(const AnimationSM* _animation) { links.erase(const_cast<AnimationSM*>(_animation)); }
	};

	/// <summary>A value of the state machine that the game sets (SetBool, SetFloat...) and transitions test. Saved as an int: never reorder.</summary>
	enum class AnimationParameterType
	{
		Bool,
		Int,
		Float,
		Trigger		// true once, until a transition uses it
	};

	/// <summary>How a condition compares its parameter (saved as an int: never reorder).</summary>
	enum class AnimationTest
	{
		IsTrue,		// Bool and Trigger
		IsFalse,
		Equal,		// Int and Float
		NotEqual,
		Greater,
		Less
	};

	struct AnimationParameter
	{
		std::string		name;
		int				type		= static_cast<int>(AnimationParameterType::Bool);
		float			value		= 0.f;		// value when the level starts
	};

	struct AnimationCondition
	{
		std::string		parameter;
		int				test		= static_cast<int>(AnimationTest::IsTrue);
		float			value		= 0.f;
	};

	/// <summary>An arrow of the state machine: from one animation to another when all its conditions are true.</summary>
	struct AnimationTransition
	{
		inline static const std::string ANY = "*";	// "from" of a transition that leaves any animation

		std::string							from;
		std::string							to;
		std::vector<AnimationCondition>		conditions;			// all must be true (none: always)
		bool								waitForEnd	= false;	// only once the current animation reached its last frame
	};

	/// <summary>
	/// The state machine made in the Animation Graph editor: parameters, transitions and where each box is drawn. Saved in scenes
	/// as one text (property "graph").
	/// </summary>
	struct AnimationGraph
	{
		std::vector<AnimationParameter>			parameters;
		std::vector<AnimationTransition>		transitions;
		std::map<std::string, FVector2>			positions;		// box of each animation in the editor ("*": the Any box)

		static std::string Encode(const AnimationGraph& _graph);
		static AnimationGraph Decode(const std::string& _text);

		/// <returns>The parameter of this name, or null.</returns>
		const AnimationParameter* FindParameter(const std::string& _name) const;
	};

	/// <summary>
	/// Component that manages animations via a state machine with automated conditional transitions.
	/// The transitions are made in the Animation Graph editor (parameters + conditions) or in code (AddAnimation links).
	/// </summary>
	class AnimationComponentSM : public Component
	{
		AnimationSM*												current		= nullptr;
		std::unordered_map<String, std::unique_ptr<AnimationSM>>	animations;
		std::vector<String>	appliedClips;	// names of the animations made from clips, so they can be replaced
		bool				clipStarted	= false;
		std::unordered_map<std::string, float>	values;	// parameters while playing

	public:
		/// <summary>Animations described by plain values (Inspector, Animation Editor, scenes). Call ApplyClips after changing them.</summary>
		std::vector<AnimationClip>	clips;

		/// <summary>Name of the clip that is the current animation when the level starts (empty or unknown: the first clip).</summary>
		String				startAnimation;

		/// <summary>Starts the clip animation when the level starts.</summary>
		Bool				playOnBeginPlay	= false;

		/// <summary>The state machine made in the Animation Graph editor. Call ResetParameters after changing its parameters.</summary>
		AnimationGraph		graph;

		/// <summary>(Re)creates the animations described by clips on the sprite of the owner and makes startAnimation the current one.</summary>
		void ApplyClips();

		// ---- Parameters: what the transitions of the graph test. A name the graph does not have is added (with a warning).

		void SetBool(const std::string& _name, const bool& _value);
		void SetInt(const std::string& _name, const int& _value);
		void SetFloat(const std::string& _name, const float& _value);
		/// <summary>True until a transition that tests it is taken.</summary>
		void SetTrigger(const std::string& _name);
		void ResetTrigger(const std::string& _name);
		float GetParameter(const std::string& _name) const;
		bool GetBool(const std::string& _name) const { return GetParameter(_name) != 0.f; }

		/// <summary>Every parameter back to the value it starts with.</summary>
		void ResetParameters();

		/// <summary>Goes to an animation now, whatever the transitions say.</summary>
		void PlayAnimation(const String& _name);

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

		void BeginPlay() override;

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

	private:
		void SetValue(const std::string& _name, const float& _value, const AnimationParameterType& _type);
		bool Passes(const AnimationTransition& _transition) const;
		void TakeGraphTransition();	// the first transition of the graph whose conditions are true, if any
		void SwitchTo(AnimationSM* _next);
	};

}
