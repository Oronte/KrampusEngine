#pragma once
#include "UI/LabelWidget.h"
#include "Player.h"

using namespace Krampus;

class ScoreLabelWidget : public LabelWidget
{
	Player*		player	= nullptr;
public:
	ScoreLabelWidget(Level* _level, Player* _player);

	virtual void Tick(const Float& _deltaTime) override;
};

