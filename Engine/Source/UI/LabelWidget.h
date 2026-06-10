#pragma once
#include "Widget.h"
#include "Components/LabelComponent.h"

namespace Krampus
{

	class LabelWidget : public Widget
	{
	protected:
		LabelComponent*		label	= nullptr;

	public:
        inline void SetString(const String& _string)
        {
            label->SetString(_string);
        }
        inline String GetString() const
        {
            return label->GetString();
        }

        inline void SetFont(const String& _fileName, const FontExtensionType& _type)
        {
            label->SetFont(_fileName, _type);
        }
        inline const Font& GetFont() const
        {
            return label->GetFont();
        }

        inline void SetCharacterSize(const UInt& _size)
        {
            label->SetCharacterSize(_size);
        }
        inline UInt GetCharacterSize() const
        {
            return label->GetCharacterSize();
        }

        inline void SetFillColor(const Color& _color)
        {
            label->SetFillColor(_color);
        }
        inline Color GetFillColor() const
        {
            return label->GetFillColor();
        }

        inline void SetOutlineColor(const Color& _color)
        {
            label->SetOutlineColor(_color);
        }
        inline Color GetOutlineColor() const
        {
            return label->GetOutlineColor();
        }

        inline void SetOutlineThickness(const Float& _thickness)
        {
            label->SetOutlineThickness(_thickness);
        }
        inline Float GetOutlineThickness() const
        {
            return label->GetOutlineThickness();
        }

        inline void SetStyle(const UInt& _style)
        {
            label->SetStyle(_style);
        }
        inline UInt GetStyle() const
        {
            return label->GetStyle();
        }

        inline void SetLetterSpacing(const Float& _spacing)
        {
            label->SetLetterSpacing(_spacing);
        }
        inline Float GetLetterSpacing() const
        {
            return label->GetLetterSpacing();
        }

        inline void SetLineSpacing(const Float& _spacing)
        {
            label->SetLineSpacing(_spacing);
        }
        inline Float GetLineSpacing() const
        {
            return label->GetLineSpacing();
        }

        inline void SetPosition(const FVector2& _position)
        {
            label->SetPosition(_position);
        }
        inline FVector2 GetPosition() const
        {
            return label->GetPosition();
        }

        inline void SetOrigin(const FVector2& _origin)
        {
            label->SetOrigin(_origin);
        }
        inline FVector2 GetOrigin() const
        {
            return label->GetOrigin();
        }

        inline void SetScale(const FVector2& _factors)
        {
            label->SetScale(_factors);
        }
        inline FVector2 GetScale() const
        {
            return label->GetScale();
        }

        inline void SetRotation(const Angle& _angle)
        {
            label->SetRotation(_angle);
        }
        inline Angle GetRotation() const
        {
            return label->GetRotation();
        }

        inline void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position);
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
        inline Transform GetTransform() const
        {
            return Transform(GetPosition(), GetRotation(), GetScale());
        }


        inline FRect GetLocalBounds() const
        {
            return label->GetLocalBounds();
        }
        inline FRect GetGlobalBounds() const
        {
            return label->GetGlobalBounds();
        }
        inline FVector2 FindCharacterPos(const Int& _index) const
        {
            return label->FindCharacterPos(_index);
        }

		LabelWidget(Level* _level, const TextData& _data);

		virtual std::string ToString() const override;
	};

}

