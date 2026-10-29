#pragma once
#include "Component.h"
#include "Network/NetworkManager.h"

namespace Krampus
{

	/// <summary>
	/// Makes an actor exist on every machine of a network game, with no code: add it, choose what to share, and the owner's version of the
	/// actor (position, rotation, scale and any properties you list) is copied to the others.
	///
	/// - An actor placed in the scene is owned by the host. It is found on every machine by its NETWORK ID, which comes from the actor's
	///   name: give the actors you synchronise unique names (or set networkId yourself).
	/// - An actor created with NetworkManager::Spawn (or the Game function "Spawn") belongs to the player who asked for it. Only that
	///   player moves it; the others see it follow.
	/// Test IsLocalOwner() before reading the keyboard in a script: only the owner should control the actor.
	/// - CallRPC("Explode") calls the KFUNCTION Explode of this actor (or of one of its components) on every machine; the editor can do
	///   the same from an event list (NetworkSyncComponent > RpcAll / RpcOthers / RpcHost).
	/// </summary>
	class NetworkSyncComponent : public Component
	{
		struct Target
		{
			FVector2	position;
			float		rotation	= 0.f;
			FVector2	scale		= FVector2(1.f, 1.f);
		};

		bool					registered		= false;
		int						resolvedId		= 0;
		float					timer			= 0.f;
		float					idle			= 0.f;
		bool					hasTarget		= false;
		Target					target;
		Target					lastSent;
		bool					sentOnce		= false;
		std::string				parsedFor;				// the value of properties that was parsed
		std::vector<std::pair<std::string, std::string>>	parsed;		// (class, property)

	public:
		/// <summary>Identifies the actor on every machine. 0 = made from the actor's name.</summary>
		Int			networkId			= 0;
		Bool		syncPosition		= true;
		Bool		syncRotation		= true;
		Bool		syncScale			= false;
		/// <summary>How many times per second the owner sends its state.</summary>
		Float		sendRate			= 20.f;
		/// <summary>The copies glide to the new state instead of jumping.</summary>
		Bool		smoothing			= true;
		/// <summary>Send through TCP (never lost, but slower) instead of UDP.</summary>
		Bool		reliable			= false;
		/// <summary>More values to share: "ClassName.property; ClassName.property" (the actor's class or one of its components).</summary>
		String		properties;

		/// <summary>The player that owns this actor (0 = the host). Set by Spawn.</summary>
		int			ownerId				= 0;
		/// <summary>True for an actor made by Spawn (it is destroyed on every machine when the host destroys it).</summary>
		bool		spawnedByNetwork	= false;

		NetworkSyncComponent(Actor* _owner);

		/// <returns>The id of the actor on the network.</returns>
		int GetNetworkId();

		/// <summary>The id used by an actor of this name when networkId is 0.</summary>
		static int IdFromName(const std::string& _name);

		/// <returns>True on the machine that controls this actor (the one that sends its state).</returns>
		bool IsLocalOwner();

		/// <returns>The player that owns this actor (0 = the host).</returns>
		int GetOwnerId() const { return ownerId; }

		/// <summary>
		/// Calls a KFUNCTION of this actor (or of one of its components) by name on the machines of _target.
		/// _args: nothing, or the one argument of the function (NetworkManager::Args(12.5f)).
		/// </summary>
		void CallRPC(const std::string& _function, NetTarget _target = NetTarget::All, const std::vector<PropertyValue>& _args = { });

		/// <summary>Gives the actor its network identity (called by the NetworkManager for a spawned actor).</summary>
		void AssignNetworkId(int _networkId, int _ownerId);

		/// <summary>Applies a state received from the owner (called by the NetworkManager).</summary>
		void ReceiveState(const NetMessage& _message);

		virtual void Tick(const Float& _deltaTime) override;
		virtual void Deconstruct() override;
		virtual void BeginDestroy() override;
		virtual std::string ToString() const override;

	private:
		void Register();
		void SendState();
		void ParseProperties();
		IGameObject* FindObject(const std::string& _className);
	};

}
