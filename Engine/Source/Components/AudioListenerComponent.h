#pragma once
#include "Component.h"

namespace Krampus
{

	class AudioListenerComponent : public Component
	{
		Bool	topView		= false;

	public:
		inline void SetGlobalVolume(const Float& _volume)
		{
			sf::Listener::setGlobalVolume(_volume);
		}
		inline Float GetGlobalVolume() const
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

		AudioListenerComponent(Actor* _owner, const Bool& _topView = false);

		virtual void BeginPlay() override;
		virtual void Tick(const Float& _deltaTime) override;

	private:
		void UpdateListener();

	public:
		virtual std::string ToString() const override;
	};

}

