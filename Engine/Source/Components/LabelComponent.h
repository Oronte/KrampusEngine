#pragma once
#include "Components/Component.h"
#include "Graphics/Ressources/Text.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/Drawable.h"

namespace Krampus
{
	class LabelComponent : public Component, public IDrawable, public IDebuggable
	{
		Text    text;

        Event<FVector2>::ListenerHandle onOwnerMoveHandle;
        Event<Angle>::ListenerHandle onOwnerRotateHandle;
        Event<FVector2>::ListenerHandle onOwnerScaleHandle;

	public:
        inline void SetString(const String& _string)
        {
            text.SetString(_string);
        }
        inline String GetString() const
        {
            return text.GetString();
        }

        inline void SetFont(const String& _fileName, const FontExtensionType& _type)
        {
            text.SetFont(_fileName, _type);
        }
        inline const Font& GetFont() const
        {
            return text.GetFont();
        }

        inline void SetCharacterSize(const UInt& _size)
        {
            text.SetCharacterSize(_size);
        }
        inline UInt GetCharacterSize() const
        {
            return text.GetCharacterSize();
        }

        inline void SetFillColor(const Color& _color)
        {
            text.SetFillColor(_color);
        }
        inline Color GetFillColor() const
        {
            return text.GetFillColor();
        }

        inline void SetOutlineColor(const Color& _color)
        {
            text.SetOutlineColor(_color);
        }
        inline Color GetOutlineColor() const
        {
            return text.GetOutlineColor();
        }

        inline void SetOutlineThickness(const Float& _thickness)
        {
            text.SetOutlineThickness(_thickness);
        }
        inline Float GetOutlineThickness() const
        {
            return text.GetOutlineThickness();
        }

        inline void SetStyle(const UInt& _style)
        {
            text.SetStyle(_style);
        }
        inline UInt GetStyle() const
        {
            return text.GetStyle();
        }

        inline void SetLetterSpacing(const Float& _spacing)
        {
            text.SetLetterSpacing(_spacing);
        }
        inline Float GetLetterSpacing() const
        {
            return text.GetLetterSpacing();
        }

        inline void SetLineSpacing(const Float& _spacing)
        {
            text.SetLineSpacing(_spacing);
        }
        inline Float GetLineSpacing() const
        {
            return text.GetLineSpacing();
        }

        inline void SetPosition(const FVector2& _position)
        {
            text.SetPosition(_position);
        }
        inline FVector2 GetPosition() const
        {
            return text.GetPosition();
        }

        inline void SetOrigin(const FVector2& _origin)
        {
            text.SetOrigin(_origin);
        }
        inline FVector2 GetOrigin() const
        {
            return text.GetOrigin();
        }

        inline void SetScale(const FVector2& _factors)
        {
            text.SetScale(_factors);
        }
        inline FVector2 GetScale() const
        {
            return text.GetScale();
        }

        inline void SetRotation(const Angle& _angle)
        {
            text.SetRotation(_angle);
        }
        inline Angle GetRotation() const
        {
            return Angle(text.GetRotation());
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
            return text.GetLocalBounds();
        }
        inline FRect GetGlobalBounds() const
        {
            return text.GetGlobalBounds();
        }
        inline FVector2 FindCharacterPos(const Int& _index) const
        {
            return text.FindCharacterPos(_index);
        }

		LabelComponent(Actor* _owner, const TextData& _data);

	private:
		void Draw(Window& _window) override;
        void DrawDebug() override;

    public:
        virtual void Construct() override;
        virtual void Deconstruct() override;
        virtual void SetZOrder(const uint8_t& _zOrder) override;
        virtual void SetZOrder(const ZOrder& _zOrder) override;

    public:
        virtual std::string ToString() const override;
    };

}

