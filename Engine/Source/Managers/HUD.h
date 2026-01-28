#pragma once
#include "UI/Widget.h"

namespace Krampus
{

	class HUD
	{
		std::vector<std::unique_ptr<Widget>> widgets;

	public:
		inline const std::vector<std::unique_ptr<Widget>>& GetWidgets() const noexcept
		{
			return widgets;
		}
		template <typename Type, IS_BASE_OF(Widget, Type)>
		inline std::vector<Type*> GetAllWidgetOfClass()
		{
			std::vector<Type*> _finalVector;

			for (const std::unique_ptr<Widget>& _widget : widgets)
			{
				if (Type* _castedWidget = Cast<Type>(_widget.get()))
					_finalVector.push_back(_castedWidget);
			}

			return _finalVector;
		}
		template <typename Type, typename ...Args, IS_BASE_OF(Widget, Type)>
		inline Type* CreateWidget(Args&&... _args)
		{
			std::unique_ptr<Type> _widget = std::make_unique<Type>(std::forward<Args>(_args)...);
			Type* _rawWidget = _widget.get();
			widgets.push_back(std::move(_widget));
			_rawWidget->Construct();
			return _rawWidget;
		}

		inline void DeleteWidget(Widget* _toDelete)
		{
			std::erase_if(widgets, [&](const std::unique_ptr<Widget>& _widget)
				{
					return _widget.get() == _toDelete;
				});
		}

		HUD() = default;

	public:
		void BeginPlay();
		void Update(const float& _deltaTime);
		void BeginDestroy();
	};


}
