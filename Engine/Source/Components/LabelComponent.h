#pragma once
#include "Components/Component.h"
#include "Graphics/Ressources/Text.h"
#include "Utilities/System/Debuggable.h"
#include "Utilities/System/Drawable.h"

namespace Krampus
{
	class LabelComponent : public Component, public IDrawable, public IDebuggable
	{
		Text text;

	public:
        inlin void SetString(const std::string& _string)
        {
            text.SetString(_string);
        }
        inlin const std::string& GetString() const
        {
            return text.GetString();
        }

        inlin void SetFont(const std::string& _fileName, const FontExtensionType& _type)
        {
            text.SetFont(_fileName, _type);
        }
        inlin const Font& GetFont() const
        {
            return text.GetFont();
        }

        inlin void SetCharacterSize(unsigned int& _size)
        {
            text.SetCharacterSize(_size);
        }
        inlin unsigned int GetCharacterSize() const
        {
            return text.GetCharacterSize();
        }

        inlin void SetFillColor(const Color& _color)
        {
            text.SetFillColor(_color);
        }
        inlin Color GetFillColor() const
        {
            return text.GetFillColor();
        }

        inlin void SetOutlineColor(const Color& _color)
        {
            text.SetOutlineColor(_color);
        }
        inlin Color GetOutlineColor() const
        {
            return text.GetOutlineColor();
        }

        inlin void SetOutlineThickness(const float& _thickness)
        {
            text.SetOutlineThickness(_thickness);
        }
        inlin float GetOutlineThickness() const
        {
            return text.GetOutlineThickness();
        }

        inlin void SetStyle(const unsigned int& _style)
        {
            text.SetStyle(_style);
        }
        inlin unsigned int GetStyle() const
        {
            return text.GetStyle();
        }

        inlin void SetLetterSpacing(const float& _spacing)
        {
            text.SetLetterSpacing(_spacing);
        }
        inlin float GetLetterSpacing() const
        {
            return text.GetLetterSpacing();
        }

        inlin void SetLineSpacing(const float& _spacing)
        {
            text.SetLineSpacing(_spacing);
        }
        inlin float GetLineSpacing() const
        {
            return text.GetLineSpacing();
        }

        inlin void SetPosition(const FVector2& _position)
        {
            text.SetPosition(_position);
        }
        inlin FVector2 GetPosition() const
        {
            return text.GetPosition();
        }

        inlin void SetOrigin(const FVector2& _origin)
        {
            text.SetOrigin(_origin);
        }
        inlin FVector2 GetOrigin() const
        {
            return text.GetOrigin();
        }

        inlin void SetScale(const FVector2& _factors)
        {
            text.SetScale(_factors);
        }
        inlin FVector2 GetScale() const
        {
            return text.GetScale();
        }

        inlin void SetRotation(const Angle& _angle)
        {
            text.SetRotation(_angle);
        }
        inlin Angle GetRotation() const
        {
            return Angle(text.GetRotation());
        }

        inlin void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position);
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
        inlin Transform GetTransform() const
        {
            return Transform(GetPosition(), GetRotation(), GetScale());
        }


        inlin FRect GetLocalBounds() const
        {
            return text.GetLocalBounds();
        }
        inlin FRect GetGlobalBounds() const
        {
            return text.GetGlobalBounds();
        }
        inlin FVector2 FindCharacterPos(const std::size_t& _index) const
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

    protected:
        virtual void Tick(const float& _deltaTime) override;
    };

}

