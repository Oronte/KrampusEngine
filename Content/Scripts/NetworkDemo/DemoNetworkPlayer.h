#pragma once
#include "Reflection/Reflection.h"
#include "Components/NetworkSyncComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/LabelComponent.h"

namespace Krampus
{

	/// <summary>
	/// The area the players of the network demo move in: what the camera shows, minus a margin.
	/// </summary>
	FRect DemoArena(Level* _level);

	/// <summary>
	/// One player of the network demo (scene DemoNetwork). Made by NetworkManager::Spawn: the machine that asked for it owns it.
	///   - Its owner moves it (ZQSD, WASD or the arrows): the NetworkSyncComponent copies the position to the others.
	///   - F / G make it bigger / smaller: "size" is listed in the NetworkSyncComponent's properties, so the others see it too.
	///   - Space calls Flash on every machine: an RPC (NetworkSyncComponent::CallRPC).
	/// Its color and name come from its owner (NetworkManager::GetPlayerName).
	/// </summary>
	class DemoNetworkPlayer : public Actor
	{
		KACTOR(DemoNetworkPlayer, Actor)

	public:
		DemoNetworkPlayer(Level* _level);

		KPROPERTY_RANGE(Float, speed, 50.f, 1500.f) = 420.f;		// pixels per second
		KPROPERTY_RANGE(Float, size, 12.f, 120.f) = 32.f;			// radius, shared with the others (NetworkSyncComponent properties)

		/// <summary>A white flash and a bounce, on every machine when it comes from CallRPC("Flash").</summary>
		KFUNCTION(Flash)
		void Flash();

		/// <returns>The color of a player, from its id.</returns>
		static Color ColorOf(int _playerId);

	protected:
		void Tick(const Float& _deltaTime) override;

	private:
		SpriteComponent*		body		= nullptr;
		LabelComponent*			nameLabel	= nullptr;
		NetworkSyncComponent*	sync		= nullptr;
		std::string				shownName;
		float					shownSize	= 0.f;
		float					flashTime	= 0.f;
		bool					spaceHeld	= true;		// true: a Space already held when the player appears does not flash
		bool					resizeHeld	= true;

		void UpdateLook();
		void UpdateControls(const float& _deltaTime);
	};

}
