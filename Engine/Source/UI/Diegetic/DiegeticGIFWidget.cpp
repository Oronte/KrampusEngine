#include "DiegeticGIFWidget.h"

Krampus::DiegeticGIFWidget::DiegeticGIFWidget(Level* _level, const CircleShapeData& _shapeData, const AnimationData& _animationData)
	: DiegeticImageWidget(_level, _shapeData)
{
	name = NAME_OF(GIFWidget);
	Init(_animationData);
}

Krampus::DiegeticGIFWidget::DiegeticGIFWidget(Level* _level, const RectangleShapeData& _shapeData, const AnimationData& _animationData)
	: DiegeticImageWidget(_level, _shapeData)
{
	name = NAME_OF(GIFWidget);
	Init(_animationData);
}

void Krampus::DiegeticGIFWidget::Play()
{
	animation->StartAnimation();
}

void Krampus::DiegeticGIFWidget::Stop()
{
	animation->StopAnimation();
}

void Krampus::DiegeticGIFWidget::Resume()
{
	animation->ResumeAnimation();
}

void Krampus::DiegeticGIFWidget::Pause()
{
	animation->PauseAnimation();
}

void Krampus::DiegeticGIFWidget::Init(const AnimationData& _data)
{
	animation = CreateComponent<AnimationComponent>();
	animation->AddAnimation(animationName, _data);
	animation->SetCurrentAnimation(animationName);
	Play();
}

std::string Krampus::DiegeticGIFWidget::ToString() const
{
	String _isPlayingText = "False";
	if (Animation* _anim = animation->GetCurrentAnimation())
		if (_anim->GetState() == Animation::State::Playing)
			_isPlayingText = "True";
	return name + " -> AnimationName = " + animationName + ", IsPlaying = " + _isPlayingText;
}
