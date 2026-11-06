#pragma once
#include "Reflection/Reflection.h"
#include "UI/Widgets/BasicWidgets.h"
#include "UI/Widgets/InputWidgets.h"
#include "Network/NetworkManager.h"

namespace Krampus
{

	/// <summary>
	/// The chat of Blackout, in the lobby and during a network game (made by BlackoutMenuController and BlackoutGameMode, never saved).
	///   Enter: a message to everybody, T: a message to your team. Enter sends it, Escape cancels.
	/// The last messages show at the bottom left and fade after a while (all of them while you type). Message BO_Chat (team or -1,
	/// name, text): every machine gets it, a team message only shows to that team.
	/// </summary>
	class BlackoutChat : public Actor
	{
	public:
		BlackoutChat(Level* _level);

		/// <summary>Makes the widgets: the bottom line of the messages at _bottomLeft, the typing line of _width under it.</summary>
		void Build(const FVector2& _bottomLeft, const float& _width);

		/// <returns>The chat of the level, or nullptr.</returns>
		static BlackoutChat* Of(Level* _level);

		/// <returns>True while the player types (the game ignores the keyboard).</returns>
		inline bool IsTyping() const { return typing; }

		/// <returns>True just after the typing line closed (the Escape or Enter that closed it must not do anything else).</returns>
		inline bool JustClosed() const { return sinceClosed < 0.3f; }

		/// <summary>A line that only this machine sees (joins, round results...).</summary>
		void AddInfo(const std::string& _text, const Color& _color);

	protected:
		void Tick(const Float& _deltaTime) override;

	private:
		struct Line
		{
			std::string	text;
			Color		color;
			float		age		= 0.f;
		};

		static constexpr int	LINES	= 6;

		std::vector<Line>							lines;
		std::vector<TextWidget*>					shown;
		TextEntryWidget*							entry			= nullptr;
		TextWidget*									prompt			= nullptr;
		bool										typing			= false;
		bool										toTeam			= false;
		bool										enterHeld		= true;
		bool										teamHeld		= true;
		float										sinceClosed		= 1.f;
		Event<const NetMessage&>::ListenerHandle	messageHandle;
		Event<const std::string&>::ListenerHandle	submitHandle;

		void Open(const bool& _team);
		void Close();
		void Refresh();
	};

}
