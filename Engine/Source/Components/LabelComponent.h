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
        inline void SetString(const std::string& _string)
        {
            text.SetString(_string);
        }
        inline const std::string& GetString() const
        {
            return text.GetString();
        }

        inline void SetFont(const std::string& _fileName, const FontExtensionType& _type)
        {
            text.SetFont(_fileName, _type);
        }
        inline const Font& GetFont() const
        {
            return text.GetFont();
        }

        inline void SetCharacterSize(unsigned int& _size)
        {
            text.SetCharacterSize(_size);
        }
        inline unsigned int GetCharacterSize() const
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

        inline void SetOutlineThickness(const float& _thickness)
        {
            text.SetOutlineThickness(_thickness);
        }
        inline float GetOutlineThickness() const
        {
            return text.GetOutlineThickness();
        }

        inline void SetStyle(const unsigned int& _style)
        {
            text.SetStyle(_style);
        }
        inline unsigned int GetStyle() const
        {
            return text.GetStyle();
        }

        inline void SetLetterSpacing(const float& _spacing)
        {
            text.SetLetterSpacing(_spacing);
        }
        inline float GetLetterSpacing() const
        {
            return text.GetLetterSpacing();
        }

        inline void SetLineSpacing(const float& _spacing)
        {
            text.SetLineSpacing(_spacing);
        }
        inline float GetLineSpacing() const
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
        inline FVector2 FindCharacterPos(const std::size_t& _index) const
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

