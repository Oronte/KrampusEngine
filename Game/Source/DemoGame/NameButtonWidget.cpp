#include "NameButtonWidget.h"
#include "Managers/AudioManager.h"

NameButtonWidget::NameButtonWidget(Level* _level, const RectangleShapeData& _data, const String& _label)
	: ButtonWidget(_level, _data)
{
	label = CreateComponent<LabelComponent>(TextData(_label));
	label->SetZOrder(210);
	sound = M_AUDIO.CreateSound("_internal/MsgLogNotif", AudioExtensionType::MP3);
}

void NameButtonWidget::Construct()
{
	Super::Construct();

	hoverHandle = onHover.AddListener(this, &NameButtonWidget::OnHover);
	unhoverHandle = onUnhover.AddListener(this, &NameButtonWidget::OnUnhover);
}

void NameButtonWidget::OnUnhover()
{
	SetActorScale(FVector2(1.0f));
	sound.Play();
}

void NameButtonWidget::OnHover()
{
	SetActorScale(FVector2(1.15f));
	sound.Play();
}
