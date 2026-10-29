#pragma once
#include "Core/GameObject.h"

namespace Krampus
{

	using PropertyValue = std::variant<bool, int, float, std::string, FVector2, Color>;

	/// <summary>
	/// One call of an event list: "when the event happens, call this function of this object with this argument".
	/// target: "" the object that owns the list, "A:Name" an actor of the level, "W:Name" a HUD widget, "Game" the game itself (open scene, quit).
	/// component: "" the actor / widget itself, else the class name of one of its components. function: a reflected function (KFUNCTION).
	/// </summary>
	struct EventCall
	{
		std::string		target;
		std::string		component;
		std::string		function;
		PropertyValue	argument	= std::string();	// used when the function takes one
	};

	/// <summary>
	/// Functions to call when something happens, edited in the Inspector like a Unity event and saved in scenes.
	/// A script declares one with KPROPERTY(EventList, onDeath) and fires it with onDeath.Invoke(this).
	/// </summary>
	struct EventList
	{
		std::vector<EventCall>	calls;

		/// <summary>Written as one string so it is saved like any property.</summary>
		std::string Encode() const;
		static EventList Decode(const std::string& _text);

		/// <summary>Calls every function of the list. _owner is the object the list belongs to (it gives the level and the "" target).</summary>
		void Invoke(IGameObject* _owner) const;

		/// <summary>The object a call points at (the actor, the widget or one of their components), or nullptr if it does not exist. For target "Game", _owner itself.</summary>
		static IGameObject* Resolve(IGameObject* _owner, const EventCall& _call);
	};


}
