#include "GIFWidget.h"

Krampus::GIFWidget::GIFWidget(Level* _level, const CircleShapeData& _shapeData, const AnimationData& _animationData)
	: ImageWidget(_level, _shapeData)
{
	name = NAME_OF(GIFWidget);
	Init(_animationData);
}

Krampus::GIFWidget::GIFWidget(Level* _level, const RectangleShapeData& _shapeData, const AnimationData& _animationData)
	: ImageWidget(_level, _shapeData)
{
	name = NAME_OF(GIFWidget);
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

std::string Krampus::GIFWidget::ToString() const
{
	String _isPlayingText = "False";
	if (Animation* _anim = animation->GetCurrentAnimation())
		if (_anim->GetState() == Animation::State::Playing)
			_isPlayingText = "True";
	return name + " -> AnimationName = " + animationName + ", IsPlaying = " + _isPlayingText;
}
