#include "ScoreLabelWidget.h"

ScoreLabelWidget::ScoreLabelWidget(Level* _level, Player* _player)
	: LabelWidget(_level, TextData("0")), player(_player)
{
	label->SetFillColor(Color::Blue());
}

void ScoreLabelWidget::Tick(const Float& _deltaTime)
{
	Super::Tick(_deltaTime);

	label->SetString(player->GetScore().ToString());
}
