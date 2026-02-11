#pragma once
#include "Component.h"

namespace Krampus
{

	class AudioListenerComponent : public Component
	{
		bool topView;

	public:
		inline void SetGlobalVolume(const float _volume)
		{
			sf::Listener::setGlobalVolume(_volume);
		}
		inline float GetGlobalVolume() const
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

		AudioListenerComponent(Actor* _owner, bool _topView = false);

		virtual void BeginPlay() override;
		virtual void Tick(const float& _deltaTime) override;

	private:
		void UpdateListener();
	};

}

