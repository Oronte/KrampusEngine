#pragma once
#include "ControlWidget.h"

namespace Krampus
{
	/// <summary>
	/// A rectangle that holds child widgets (AddChild, or a parent in the Hierarchy). The boxes below place them every frame:
	/// moving a child by hand does nothing there. The children are taken in the order of GetChildren; inactive ones are skipped.
	/// </summary>
	class PanelWidget : public ControlWidget
	{
	public:
		/// <summary>Space between the border and the children (before the scale).</summary>
		Float			padding		= 10.f;
		/// <summary>Space between two children (before the scale).</summary>
		Float			spacing		= 8.f;
		/// <summary>Across the box: 0 start (left / top), 1 center, 2 end (right / bottom).</summary>
		Int				alignment	= 1;
		/// <summary>The size follows the children (with the padding) instead of the size property.</summary>
		Bool			fitContent	= false;

		PanelWidget(Level* _level, const FVector2& _size);

	protected:
		/// <summary>The children that are placed: active, not being destroyed.</summary>
		std::vector<Widget*> LaidOutChildren() const;

		/// <summary>Moves _child so that its bounds start at _topLeft (nothing is broadcast when it is already there).</summary>
		static void PlaceAt(Widget* _child, const FVector2& _topLeft);

		/// <summary>fitContent: the size of the panel for content of this size on the screen.</summary>
		void FitTo(const FVector2& _content);
	};

	/// <summary>
	/// A panel with a background where the children stay where they are put (their position in the canvas is kept when it moves).
	/// </summary>
	class CanvasWidget : public PanelWidget
	{
	public:
		CanvasWidget(Level* _level);
	};

	/// <summary>
	/// The children one after the other: left to right (HorizontalBoxWidget) or top to bottom (VerticalBoxWidget).
	/// </summary>
	class BoxWidget : public PanelWidget
	{
	public:
		/// <summary>Top to bottom when true, left to right otherwise.</summary>
		Bool			vertical	= false;
		/// <summary>Along the box: 0 start, 1 center, 2 end (where the row of children is when it is shorter than the box).</summary>
		Int				justify		= 0;

		BoxWidget(Level* _level, const bool& _vertical);

	protected:
		void Layout() override;
	};

	class HorizontalBoxWidget : public BoxWidget
	{
	public:
		HorizontalBoxWidget(Level* _level);
	};

	class VerticalBoxWidget : public BoxWidget
	{
	public:
		VerticalBoxWidget(Level* _level);
	};

	/// <summary>
	/// The children in rows of `columns` cells, each one centered in its cell.
	/// </summary>
	class GridWidget : public PanelWidget
	{
	public:
		Int				columns		= 3;
		/// <summary>Size of a cell before the scale; 0 on an axis: the biggest child.</summary>
		FVector2		cellSize;

		GridWidget(Level* _level);

	protected:
		void Layout() override;
	};

	/// <summary>
	/// The children top to bottom, scrolled with the mouse wheel: what is out of the box is not drawn nor clicked.
	/// </summary>
	class ScrollBoxWidget : public PanelWidget
	{
	public:
		/// <summary>How far the content is scrolled down, in pixels before the scale (kept between 0 and the end).</summary>
		Float			scroll			= 0.f;
		/// <summary>Pixels per notch of the wheel.</summary>
		Float			scrollSpeed		= 40.f;
		Bool			showScrollBar	= true;
		Color			scrollBarColor	= Color(255, 255, 255, 110);

		ScrollBoxWidget(Level* _level);
		~ScrollBoxWidget() override;

		std::optional<FRect> GetChildClip() const override { return GetRect(); }

		/// <summary>How far it can scroll (0 when everything fits).</summary>
		float GetMaxScroll() const { return maxScroll; }

	protected:
		void Layout() override;
		void DrawControl(Window& _window) override;
		void OnWheel(const float& _delta) override;

	private:
		float			maxScroll		= 0.f;
		float			contentHeight	= 0.f;
	};

	/// <summary>
	/// An empty space of `size` (draws nothing): to push children apart in a box.
	/// </summary>
	class SpacerWidget : public ControlWidget
	{
	public:
		SpacerWidget(Level* _level);
	};
}
