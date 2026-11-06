#pragma once
#include "Reflection/Reflection.h"
#include "UI/Widgets/BasicWidgets.h"

namespace Krampus
{

	class BlackoutButton;

	/// <summary>
	/// The keys of Blackout, chosen by the player (main menu > Touches, BlackoutKeysMenu) and kept in Content/Saves/BlackoutKeys.cfg
	/// (one "action = scancode" per line). Scancodes are places on the keyboard, not letters: the defaults are the same keys on an
	/// AZERTY and a QWERTY keyboard (W of QWERTY = Z of AZERTY). Fixed, whatever is chosen: the arrows also move, left click shoots,
	/// right click throws a grenade, Escape goes back.
	/// </summary>
	class BlackoutKeys
	{
	public:
		enum Action { MoveUp, MoveDown, MoveLeft, MoveRight, Walk, Reload, Grenade, Flash, Smoke, Use, Shop, Primary, Sidearm, ChatAll, ChatTeam, Ping, COUNT };

		/// <returns>What the action does, in French ("Avancer").</returns>
		static std::string Label(const int& _action);
		static sf::Keyboard::Scancode Get(const int& _action);
		/// <summary>Gives _key to _action; the action that had it takes the old key of _action (no key does two things). Saved at once.</summary>
		static void Set(const int& _action, const sf::Keyboard::Scancode& _key);
		/// <summary>Every action back to its default key. Saved at once.</summary>
		static void ResetAll();
		/// <returns>True while the key of _action is down (the keyboard of the whole computer: check the focus of the window).</returns>
		static bool Down(const int& _action);
		/// <returns>The name of the key of _action on this keyboard ("Z" on AZERTY for the default Avancer).</returns>
		static std::string KeyName(const int& _action);
		static std::string KeyName(const sf::Keyboard::Scancode& _key);

	private:
		static sf::Keyboard::Scancode Default(const int& _action);
		static std::array<sf::Keyboard::Scancode, COUNT>& Keys();		// read from the file the first time
		static void Save();
	};

	/// <summary>
	/// The menu that changes the keys: a line per action (its name, a button with its key). Click a key, then press the new one
	/// (Escape: no change). "Par defaut" puts every key back, "Retour" closes it. While it is open, the other widgets of the HUD are hidden
	/// (and come back after). Made by BlackoutMenuController (command "keys"), never saved.
	/// </summary>
	class BlackoutKeysMenu : public Actor
	{
	public:
		BlackoutKeysMenu(Level* _level);

		void Open();
		void Close();
		inline bool IsOpen() const { return open; }
		/// <summary>"keys" (open it), and the buttons of the menu: "key<n>" (wait for the new key of action n), "keysdefault", "keysback".
		/// Run at the next Tick (a click arrives while the HUD walks its widgets).</summary>
		void Command(const std::string& _command);

	protected:
		void Tick(const Float& _deltaTime) override;

	private:
		bool						open		= false;
		bool						built		= false;
		int							listening	= -1;		// the action waiting for its new key
		bool						released	= false;	// every key was up once since the click
		bool						escapeHeld	= true;
		std::vector<Widget*>		hidden;					// the widgets of the scene hidden while the menu is open
		std::vector<Widget*>		widgets;				// the menu's own
		ImageWidget*				panel		= nullptr;
		TextWidget*					hint		= nullptr;
		std::vector<BlackoutButton*>	keyButtons;

		std::string					pending;				// the command of the last click

		void RunCommand(const std::string& _command);
		void Build();
		void Refresh();
		TextWidget* MakeText(const FVector2& _position, const unsigned& _size, const std::string& _text, const float& _align);
	};

}
