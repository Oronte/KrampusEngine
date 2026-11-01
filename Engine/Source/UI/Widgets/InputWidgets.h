#pragma once
#include "ControlWidget.h"
#include "Reflection/EventList.h"

namespace Krampus
{
	/// <summary>
	/// A value between minValue and maxValue, dragged with the mouse (or the wheel over it).
	/// </summary>
	class SliderWidget : public ControlWidget
	{
		Float							value			= 0.5f;
		bool							dragging		= false;

	public:
		Float							minValue		= 0.f;
		Float							maxValue		= 1.f;
		/// <summary>The value goes by steps of this (0: any value).</summary>
		Float							step			= 0.f;
		/// <summary>Bottom (min) to top (max) when true, left to right otherwise.</summary>
		Bool							vertical		= false;
		Color							trackColor		= Color(60, 60, 72);
		Color							fillColor		= Color(80, 160, 255);
		Color							handleColor		= Color(240, 240, 245);
		/// <summary>Diameter of the handle before the scale.</summary>
		Float							handleSize		= 22.f;

		/// <summary>Broadcast with the new value when the player changes it (not when the code or a scene sets it).</summary>
		Event<float>					onValueChanged;
		EventList						onValueChangedCalls;
		/// <summary>Broadcast when the player lets the handle go.</summary>
		Event<float>					onReleased;

		SliderWidget(Level* _level);

		Float GetValue() const { return value; }
		/// <summary>Clamped to [minValue, maxValue] and put on a step. _notify: broadcast onValueChanged if it changed.</summary>
		void SetValue(const float& _value, const bool& _notify = false);
		/// <summary>0 at minValue, 1 at maxValue.</summary>
		float GetRatio() const;

	protected:
		void DrawControl(Window& _window) override;
		void OnMouseDown() override;
		void OnMouseMove() override;
		void OnMouseUp() override;
		void OnWheel(const float& _delta) override;

	private:
		void SetFromMouse();
	};

	/// <summary>
	/// A bar filled from 0 to 1 (health, loading...).
	/// </summary>
	class ProgressBarWidget : public ControlWidget
	{
		Float							value			= 0.6f;

	public:
		Color							fillColor		= Color(90, 200, 110);
		/// <summary>0 left to right, 1 right to left, 2 bottom to top, 3 top to bottom.</summary>
		Int								direction		= 0;
		/// <summary>Writes the percent in the middle.</summary>
		Bool							showText		= false;
		Color							textColor		= Color(255, 255, 255);
		Int								characterSize	= 18;

		ProgressBarWidget(Level* _level);

		Float GetValue() const { return value; }
		void SetValue(const float& _value) { value = std::clamp(_value, 0.f, 1.f); }

	protected:
		void DrawControl(Window& _window) override;
	};

	/// <summary>
	/// A box that is checked or not, with a text on its right. Boxes of the same group (not empty) are radio buttons:
	/// checking one unchecks the others of the HUD.
	/// </summary>
	class CheckBoxWidget : public ControlWidget
	{
		Bool							checked			= false;

	public:
		String							text			= String("Check box");
		String							group;
		Color							boxColor		= Color(50, 50, 62);
		Color							checkColor		= Color(80, 160, 255);
		Color							textColor		= Color(235, 235, 240);
		Int								characterSize	= 20;

		/// <summary>Broadcast with the new state when the player clicks it, or another box of its group takes the check.</summary>
		Event<bool>						onChanged;
		EventList						onCheckedCalls;
		EventList						onUncheckedCalls;

		CheckBoxWidget(Level* _level);

		Bool IsChecked() const { return checked; }
		/// <summary>_notify: broadcast onChanged (and the event lists) if it changed. Checking unchecks the group.</summary>
		void SetChecked(const bool& _checked, const bool& _notify = false);

	protected:
		void DrawControl(Window& _window) override;
		void OnMouseDown() override;
	};

	/// <summary>
	/// A line of text the player types in: a click gives it the keyboard, Enter sends it (onSubmit), Escape or a click
	/// elsewhere leaves it.
	/// </summary>
	class TextEntryWidget : public ControlWidget
	{
		sf::String						value;
		bool							focused			= false;
		sf::Clock						caretClock;

	public:
		String							placeholder		= String("Type here...");
		/// <summary>Characters at most (0: no limit).</summary>
		Int								maxLength		= 0;
		/// <summary>Shows a star for each character.</summary>
		Bool							password		= false;
		Int								characterSize	= 22;
		Color							textColor		= Color(240, 240, 245);
		Color							placeholderColor = Color(140, 140, 150);
		Color							focusColor		= Color(80, 160, 255);

		/// <summary>Broadcast with the whole text each time the player changes it.</summary>
		Event<const std::string&>		onTextChanged;
		/// <summary>Broadcast with the text when the player presses Enter.</summary>
		Event<const std::string&>		onSubmit;
		EventList						onTextChangedCalls;
		EventList						onSubmitCalls;

		TextEntryWidget(Level* _level);

		/// <summary>The text, in UTF-8.</summary>
		std::string GetText() const { return ToUtf8(value); }
		void SetText(const std::string& _text);

		bool IsFocused() const { return focused; }
		void SetFocused(const bool& _focused);

	protected:
		void DrawControl(Window& _window) override;
		void OnMouseDown() override;
		void OnText(const std::uint32_t& _character) override;

	public:
		void Deconstruct() override;
	};

	/// <summary>
	/// One choice in a list that opens under it (options separated by ';').
	/// </summary>
	class DropdownWidget : public ControlWidget
	{
		Int								selected		= 0;
		bool							open			= false;
		uint8_t							closedZOrder	= 50;

	public:
		Color							itemColor		= Color(44, 44, 56);
		Color							hoverColor		= Color(70, 110, 180);
		Color							textColor		= Color(240, 240, 245);
		Int								characterSize	= 20;

		/// <summary>Broadcast with the index and the text of the option the player picks.</summary>
		Event<int, const std::string&>	onSelectionChanged;
		EventList						onSelectionChangedCalls;

		DropdownWidget(Level* _level);

		/// <summary>"A;B;C" (saved as is).</summary>
		void SetOptions(const std::string& _options);
		const std::string& GetOptionsText() const { return optionsText; }
		const std::vector<std::string>& GetOptions() const { return options; }

		Int GetSelected() const { return selected; }
		std::string GetSelectedText() const;
		void SetSelected(const int& _index, const bool& _notify = false);

		bool IsOpen() const { return open; }
		void SetOpen(const bool& _open);

	protected:
		void DrawControl(Window& _window) override;
		void OnMouseDown() override;

	public:
		void Deconstruct() override;

	private:
		std::string						optionsText;
		std::vector<std::string>		options;
		int ItemUnderMouse() const;		// -1: none
	};
}
