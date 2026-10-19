#pragma once
#include "UI/Diegetic/DiegeticLabelWidget.h"
#include "Player.h"

using namespace Krampus;

class ScoreLabelWidget : public DiegeticLabelWidget
{
	Player*		player	= nullptr;
public:
	ScoreLabelWidget(Level* _level, Player* _player);

	virtual void Tick(const Float& _deltaTime) override;
};

