#pragma once
#include "Component.h"
#include "UI/Widgets/WidgetComponents/WidgetComponent.h"
#include "Graphics/Resources/Animation.h"

namespace Krampus
{

	/// <summary>
	/// Plays sprite-sheet animations on the Image component of the same owner (SpriteComponent / ImageWidgetComponent).
	/// The clips (edited in the Animation Editor) are built on the first frame that runs, then the start animation plays
	/// if playOnBeginPlay. Shared by AnimationComponent (world) and AnimationWidgetComponent (HUD).
	/// </summary>
	template <typename Base, typename Image>
	class BasicAnimation : public Base
	{
		Animation*												current		= nullptr;
		std::unordered_map<String, std::unique_ptr<Animation>>	animations;
		std::vector<String>										appliedClips;	// names of the animations made from clips, so they can be replaced
		bool													clipStarted	= false;

	protected:
		explicit BasicAnimation(typename Base::OwnerType* _owner) : Base(_owner) {}

	public:
		std::vector<AnimationClip>	clips;
		String						startAnimation;
		Bool						playOnBeginPlay	= false;

		/// <summary>Rebuilds the animations from clips (after an edit) and restarts on the next frame.</summary>
		void ApplyClips();

		/// <summary>Adds an animation on the owner's Image component; ignored if the name exists or there is no Image.</summary>
		virtual void AddAnimation(const String& _name, const AnimationData& _data);

		/// <summary>Switches to another animation and shows its first frame (the previous one stops).</summary>
		void SetCurrentAnimation(const String& _name);
		[[nodiscard]] Animation* GetCurrentAnimation() const { return current; }
		/// <returns>nullptr (and an error in the log) when there is no animation with that name.</returns>
		[[nodiscard]] Animation* GetAnimation(const String& _name) const;

		void StartAnimation()	{ if (current) current->Start(); }
		void StopAnimation()	{ if (current) current->Stop(); }
		void ResumeAnimation()	{ if (current) current->Resume(); }
		void PauseAnimation()	{ if (current) current->Pause(); }

		void Tick(const Float& _deltaTime) override;
		void Deconstruct() override;
		std::string ToString() const override;
	};

	class SpriteComponent;
	class ImageWidgetComponent;
	extern template class BasicAnimation<Component, SpriteComponent>;
	extern template class BasicAnimation<WidgetComponent, ImageWidgetComponent>;

}
