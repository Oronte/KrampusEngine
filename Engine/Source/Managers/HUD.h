#pragma once
#include "UI/Widgets/Widget.h"

namespace Krampus
{

	/// <summary>
	/// Heads-Up Display (HUD) manager managing 2D UI screen widgets, view projections, and UI update cycles.
	/// </summary>
	class HUD
	{
		std::vector<std::unique_ptr<Widget>>	widgets;
		std::vector<std::unique_ptr<Widget>>	pendingDeleteWidgets;

		sf::View								view = sf::View(FVector2(1920, 1080) / 2.0f, FVector2(1920, 1080));

	public:
		/// <summary>
		/// Gets a const reference to all managed screen widgets.
		/// </summary>
		/// <returns>Vector of unique pointers to Widget.</returns>
		inline const std::vector<std::unique_ptr<Widget>>& GetWidgets() const noexcept
		{
			return widgets;
		}

		/// <summary>
		/// Retrieves all active widgets matching a specific Widget derived type.
		/// </summary>
		/// <typeparam name="Type">Widget derived class.</typeparam>
		/// <returns>Vector of raw pointers to matching widgets.</returns>
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

		/// <summary>
		/// Constructs and adds a new widget of the specified type to the HUD.
		/// </summary>
		/// <typeparam name="Type">Widget class to instantiate.</typeparam>
		/// <param name="_args">Arguments forwarded to widget constructor.</param>
		/// <returns>Pointer to newly created Widget.</returns>
		template <typename Type, typename ...Args, IS_BASE_OF(Widget, Type)>
		inline Type* CreateWidget(Args&&... _args)
		{
			std::unique_ptr<Type> _widget = std::make_unique<Type>(std::forward<Args>(_args)...);
			Type* _rawWidget = _widget.get();
			widgets.push_back(std::move(_widget));
			_rawWidget->Construct();
			return _rawWidget;
		}

		/// <summary>
		/// Marks a widget for deferred deletion at the end of the frame.
		/// </summary>
		/// <param name="toMark">Pointer to Widget to remove.</param>
		inline void MarkForDeleteWidget(Widget* toMark)
		{
			auto it = std::find_if(
				widgets.begin(),
				widgets.end(),
				[&](const std::unique_ptr<Widget>& actor)
				{
					return actor.get() == toMark;
				});

			if (it == widgets.end())
				return;

			pendingDeleteWidgets.push_back(std::move(*it));

			widgets.erase(it);
		}

		/// <summary>
		/// Destroys all widgets queued in the pending deletion list.
		/// </summary>
		inline void DestroyPendingDeleteWidgets()
		{
			pendingDeleteWidgets.clear();
		}

		/// <summary>
		/// Default constructor for HUD.
		/// </summary>
		HUD() = default;

	public:
		/// <summary>
		/// Gets the screen view projection used for rendering HUD widgets.
		/// </summary>
		/// <returns>sf::View for screen rendering.</returns>
		NO_DISCARD sf::View GetView() const;

		/// <summary>
		/// Sets the screen view projection for HUD widgets.
		/// </summary>
		/// <param name="_view">New sf::View.</param>
		void SetView(const sf::View& _view);

		/// <summary>
		/// Invokes BeginPlay on all managed widgets.
		/// </summary>
		void BeginPlay();

		/// <summary>
		/// Updates all active widgets each frame.
		/// </summary>
		/// <param name="_deltaTime">Elapsed time in seconds.</param>
		void Update(const Float& _deltaTime);

		/// <summary>
		/// Deconstructs all managed widgets.
		/// </summary>
		void Deconstruct();

		/// <summary>
		/// Triggers destruction hooks on all widgets.
		/// </summary>
		void BeginDestroy();
	};

}
