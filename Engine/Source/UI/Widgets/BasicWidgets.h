#pragma once
#include "Widget.h"
#include "WidgetComponents/ImageWidgetComponent.h"
#include "WidgetComponents/LabelWidgetComponent.h"

namespace Krampus
{
	/// <summary>
	/// A picture (or a colored shape) on the screen: a widget with an ImageWidgetComponent.
	/// </summary>
	class ImageWidget : public Widget
	{
	protected:
		ImageWidgetComponent*	image	= nullptr;

	public:
		ImageWidget(Level* _level, const RectangleShapeData& _data = RectangleShapeData());

		ImageWidgetComponent* GetImage() const { return image; }
	};

	/// <summary>
	/// A text on the screen: a widget with a LabelWidgetComponent.
	/// </summary>
	class TextWidget : public Widget
	{
	protected:
		LabelWidgetComponent*	label	= nullptr;

	public:
		TextWidget(Level* _level, const TextData& _data = TextData("Text"));

		LabelWidgetComponent* GetLabel() const { return label; }
		void SetText(const String& _text) { label->SetString(_text); }
	};
}
