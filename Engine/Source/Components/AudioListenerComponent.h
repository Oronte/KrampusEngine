#pragma once
#include "Component.h"

namespace Krampus
{

	class AudioListenerComponent : public Component
	{
		Bool		topView			= false;
		Bool		active			= true;

		FVector2	cachedPosition	= FVector2::Zero();
		FVector2	cachedForward	= FVector2::Zero();
		FVector2	cachedUp		= FVector2::Zero();
		Bool		dirty			= true;

	public:
		static inline void SetGlobalVolume(const Float& _volume)
		{
			sf::Listener::setGlobalVolume(_volume);
		}
		static inline Float GetGlobalVolume()
		{
			return sf::Listener::getGlobalVolume();
		}

		inline void SetCone(const sf::Listener::Cone& _cone)
		{
			sf::Listener::setCone(_cone);
		}
		inline sf::Listener::Cone GetCone() const
		{
			return sf::Listener::getCone();
		}

		void SetActive(const Bool& _active);
		inline Bool IsActive() const { return active; }

		inline void		SetTopView(const Bool& _topView)	{ topView = _topView; dirty = true; }
		inline Bool		IsTopView() const					{ return topView; }

		inline void		MarkDirty() { dirty = true; }

	public:
		AudioListenerComponent(Actor* _owner, const Bool& _topView = false);

		virtual void BeginPlay()					override;
		virtual void Tick(const Float& _deltaTime)	override;

	private:
		void UpdateListener();
		Bool HasChanged(const FVector2& _position, const FVector2& _forward, const FVector2& _up) const;

	public:
		virtual std::string ToString() const override;
	};

}
