#pragma once
#include "UI/ButtonWidget.h"
#include "Managers/LevelManager.h"

namespace Krampus::Exemple
{
	class MainMenuButton : public ButtonWidget
	{
		const FVector2 hoverScale = FVector2(1.25f);
		const FVector2 unhoverScale = FVector2::One();

		Event<>::ListenerHandle onHoverHandle;
		Event<>::ListenerHandle onUnhoverHandle;
		Event<>::ListenerHandle changeLevelHandle;

	public:
		MainMenuButton(Level* _level, const CircleShapeData& _data, const String& _text);
		MainMenuButton(Level* _level, const RectangleShapeData& _data, const String& _text);

		virtual void Construct() override;
		virtual void BeginPlay() override;
		virtual void Tick(const Float& _deltaTime) override;
		virtual void Deconstruct() override;
		virtual void BeginDestroy() override;

		template<typename LevelType = Level>
		void BindChangeLevel()
		{
			changeLevelHandle = onPress.AddListener([this]()
				{
					GetWorld()->GetLevelManager()->SetLevel<LevelType>();
				});
		}
	};
}
