#include "GIFWidget.h"

Krampus::GIFWidget::GIFWidget(Level* _level, const CircleShapeData& _shapeData, const AnimationData& _animationData)
	: ImageWidget(_level, _shapeData)
{
	Init(_animationData);
}

Krampus::GIFWidget::GIFWidget(Level* _level, const RectangleShapeData& _shapeData, const AnimationData& _animationData)
	: ImageWidget(_level, _shapeData)
{
	Init(_animationData);
}

void Krampus::GIFWidget::Play()
{
	animation->StartAnimation();
}

void Krampus::GIFWidget::Stop()
{
	animation->StopAnimation();
}

void Krampus::GIFWidget::Resume()
{
	animation->ResumeAnimation();
}

void Krampus::GIFWidget::Pause()
{
	animation->PauseAnimation();
}

void Krampus::GIFWidget::Init(const AnimationData& _data)
{
	animation = CreateComponent<AnimationComponent>();
	animation->AddAnimation(animationName, _data);
	animation->SetCurrentAnimation(animationName);
	Play();
}
