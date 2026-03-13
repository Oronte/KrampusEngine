#pragma once
#include "Widget.h"
#include "Components/LabelComponent.h"

namespace Krampus
{

	class LabelWidget : public Widget
	{
	protected:
		LabelComponent*		label	= nullptr;

	public:
		LabelWidget(Level* _level, const TextData& _data);

		virtual std::string ToString() const override;
	};

}

