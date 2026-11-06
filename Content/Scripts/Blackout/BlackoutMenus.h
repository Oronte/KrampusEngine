#pragma once
#include "Reflection/Reflection.h"
#include "UI/Widgets/ButtonWidget.h"
#include "UI/Widgets/BasicWidgets.h"
#include "UI/Widgets/InputWidgets.h"
#include "Network/NetworkManager.h"

// The menus of Blackout (scenes BlackoutMenu and BlackoutLobby). The widgets are in the scenes (move them in the editor); a
// BlackoutMenuController finds them BY NAME and runs the command of the BlackoutButton that is clicked.

namespace Krampus
{

	/// <summary>
	/// A button with a text. A click runs its command through the BlackoutMenuController of the scene (solo, multi, quit, back, host,
	/// join, search, alpha, bravo, start, leave). A hidden button (inactive widget) does nothing.
	/// </summary>
	class BlackoutButton : public ButtonWidget
	{
		KWIDGET(BlackoutButton, ButtonWidget)

	public:
		BlackoutButton(Level* _level);

		KPROPERTY(String, text) = "Bouton";
		KPROPERTY(String, command);										// what the BlackoutMenuController does on a click
		KPROPERTY(Color, idleColor) = Color(30, 34, 46, 235);
		KPROPERTY(Color, hoverColor) = Color(60, 100, 165, 245);
		KPROPERTY(Color, selectedColor) = Color(45, 120, 75, 245);		// the chosen one of a group (gun, team)

		void Construct() override;
		void Tick(const Float& _deltaTime) override;

		/// <summary>Shows the button as the chosen one (BlackoutMenuController: the gun and the team of the player).</summary>
		void SetSelected(const bool& _selected) { selected = _selected; }
		/// <summary>Another size than 460 x 72 (the cards of the shop): the click follows it.</summary>
		void SetSize(const FVector2& _size) { imageComponent->SetSize(_size); }
		/// <summary>Size of the letters (32 by default). Call before the text is shown (the text is placed when it changes).</summary>
		void SetTextSize(const unsigned& _size) { label->SetCharacterSize(_size); shownText.clear(); }

	private:
		LabelWidgetComponent*		label		= nullptr;
		std::string					shownText;
		bool						hovered		= false;
		bool						selected	= false;
		Event<>::ListenerHandle		pressHandle;
		Event<>::ListenerHandle		hoverHandle;
		Event<>::ListenerHandle		unhoverHandle;

		void Paint();
	};

	/// <summary>
	/// Runs the menus of Blackout. One per menu scene:
	///   - BlackoutMenu (isLobby off): buttons solo (opens gameScene offline), multi (shows the choice "lan" / "steam" / "multiback":
	///     both open lobbyScene, steamLobby says which), keys (the keys menu, BlackoutKeysMenu; a "Touches" button is added under the
	///     lowest button when the scene has none), quit. Steam is started here so that an invitation accepted in Steam joins the game
	///     (then the lobby opens by itself).
	///   - BlackoutLobby (isLobby on): widgets NameEntry, AddressEntry, StatusText, InfoText, AlphaList, BravoList and the buttons
	///     host, join, search (LAN), alpha / bravo (choose a team: shared value "team_<id>"), start (host: everybody opens gameScene
	///     with the message BO_Start), leave, back. The widgets of OfflineWidgets show before a game is joined, those of OnlineWidgets after.
	///     Steam lobby: lanWidgets hide, "host" makes a Steam game, a list of the online Steam friends (friendsPosition) joins their
	///     game ("steamfriend<n>") or, once hosting, invites them; "steamoverlay" opens the invite window of the Steam overlay.
	/// </summary>
	class BlackoutMenuController : public Actor
	{
		KACTOR(BlackoutMenuController, Actor)

	public:
		void BeginPlay() override;

		BlackoutMenuController(Level* _level);

		KPROPERTY(Bool, isLobby) = false;
		KPROPERTY(String, gameScene) = "Blackout";
		KPROPERTY(String, lobbyScene) = "BlackoutLobby";
		KPROPERTY(String, menuScene) = "BlackoutMenu";
		KPROPERTY(String, offlineWidgets) = "NameLabel;NameEntry;AddressLabel;AddressEntry;HostButton;JoinButton;SearchButton";
		KPROPERTY(String, onlineWidgets) = "AlphaTitle;BravoTitle;AlphaList;BravoList;AlphaButton;BravoButton;StartButton;LeaveButton";
		KPROPERTY(String, lanWidgets) = "AddressLabel;AddressEntry;JoinButton;SearchButton";	// hidden in a Steam lobby
		KPROPERTY(FVector2, friendsPosition) = FVector2(1300.f, 760.f);	// top left of the Steam friends list (lobby)

		/// <summary>Runs a command with the controller of the level (called by BlackoutButton).</summary>
		static void Dispatch(Level* _level, const std::string& _command);

		/// <summary>Stored, run in Tick: a click arrives while the HUD walks its widgets (no widget may show / hide / spawn then).</summary>
		void Command(const std::string& _command);

		/// <summary>How the lobby plays: LAN (address, LAN search) or Steam (friends list, invitations). Chosen in the main menu.</summary>
		static bool steamLobby;

	protected:
		void Tick(const Float& _deltaTime) override;

	private:
		/// <summary>A line of the Steam friends list: name, what he does, a button (join his game / invite him).</summary>
		struct FriendRow
		{
			TextWidget*		name	= nullptr;
			TextWidget*		state	= nullptr;
			class BlackoutButton*	button	= nullptr;
		};
		static constexpr int	FRIEND_ROWS		= 5;
		static constexpr float	FRIEND_STEP		= 50.f;

		std::vector<std::string>				pending;
		// Main menu: the buttons of the scene, and those of the choice LAN / Steam / Retour shown by "multi".
		std::vector<Widget*>					mainButtons;
		std::vector<Widget*>					modeButtons;
		bool									choosingMode	= false;
		bool									escapeHeld		= true;
		// Lobby, Steam: the friends list.
		std::vector<Widget*>					friendWidgets;
		std::vector<FriendRow>					friendRows;
		std::vector<SteamFriend>				friends;		// shown in the rows, in order
		std::map<std::uint64_t, float>			invited;		// friend -> seconds left of "Invite !"
		TextWidget*								friendsTitle	= nullptr;
		BlackoutButton*							overlayButton	= nullptr;
		float									friendsTimer	= 0.f;
		bool									steamReady		= false;
		std::vector<Event<const NetMessage&>::ListenerHandle>	subscriptions;
		Event<>::ListenerHandle					connectedHandle;
		Event<>::ListenerHandle					failedHandle;
		Event<>::ListenerHandle					disconnectedHandle;
		Event<int>::ListenerHandle				joinedHandle;
		Event<const NetLanGame&>::ListenerHandle	lanHandle;
		bool									leaving		= false;
		class BlackoutChat*						chat		= nullptr;
		class BlackoutKeysMenu*					keysMenu	= nullptr;	// main menu: the keys (command "keys")

		Widget* Find(const std::string& _name) const;
		void SetText(const std::string& _widget, const std::string& _text, const Color& _color = Color(230, 232, 240));
		void SetInfo(const std::string& _text, const Color& _color = Color(230, 232, 240));
		/// <summary>Shows or hides the widgets of a list "A;B;C" (those of _skip are hidden).</summary>
		void ShowList(const std::string& _names, const bool& _show, const std::string& _skip = { });
		void ApplyName();
		void OpenScene(const std::string& _scene);
		void UpdateLobby();
		void RunCommand(const std::string& _command);
		// Main menu: the choice LAN / Steam.
		void BuildModeChoice();
		void ShowModeChoice(const bool& _show);
		// Lobby, Steam.
		TextWidget* MakeText(const FVector2& _position, const unsigned& _size, const std::string& _text);
		/// <summary>Changes the text of a MakeText widget, kept left aligned.</summary>
		static void SetLeftText(TextWidget* _widget, const std::string& _text);
		void BuildFriends();
		void UpdateFriends(const float& _deltaTime);
	};

}
