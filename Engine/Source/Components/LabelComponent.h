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
        INLINE void SetString(const std::string& _string)
        {
            text.SetString(_string);
        }
        INLINE const std::string& GetString() const
        {
            return text.GetString();
        }

        INLINE void SetFont(const std::string& _fileName, const FontExtensionType& _type)
        {
            text.SetFont(_fileName, _type);
        }
        INLINE const Font& GetFont() const
        {
            return text.GetFont();
        }

        INLINE void SetCharacterSize(unsigned int& _size)
        {
            text.SetCharacterSize(_size);
        }
        INLINE unsigned int GetCharacterSize() const
        {
            return text.GetCharacterSize();
        }

        INLINE void SetFillColor(const Color& _color)
        {
            text.SetFillColor(_color);
        }
        INLINE Color GetFillColor() const
        {
            return text.GetFillColor();
        }

        INLINE void SetOutlineColor(const Color& _color)
        {
            text.SetOutlineColor(_color);
        }
        INLINE Color GetOutlineColor() const
        {
            return text.GetOutlineColor();
        }

        INLINE void SetOutlineThickness(const float& _thickness)
        {
            text.SetOutlineThickness(_thickness);
        }
        INLINE float GetOutlineThickness() const
        {
            return text.GetOutlineThickness();
        }

        INLINE void SetStyle(const unsigned int& _style)
        {
            text.SetStyle(_style);
        }
        INLINE unsigned int GetStyle() const
        {
            return text.GetStyle();
        }

        INLINE void SetLetterSpacing(const float& _spacing)
        {
            text.SetLetterSpacing(_spacing);
        }
        INLINE float GetLetterSpacing() const
        {
            return text.GetLetterSpacing();
        }

        INLINE void SetLineSpacing(const float& _spacing)
        {
            text.SetLineSpacing(_spacing);
        }
        INLINE float GetLineSpacing() const
        {
            return text.GetLineSpacing();
        }

        INLINE void SetPosition(const FVector2& _position)
        {
            text.SetPosition(_position);
        }
        INLINE FVector2 GetPosition() const
        {
            return text.GetPosition();
        }

        INLINE void SetOrigin(const FVector2& _origin)
        {
            text.SetOrigin(_origin);
        }
        INLINE FVector2 GetOrigin() const
        {
            return text.GetOrigin();
        }

        INLINE void SetScale(const FVector2& _factors)
        {
            text.SetScale(_factors);
        }
        INLINE FVector2 GetScale() const
        {
            return text.GetScale();
        }

        INLINE void SetRotation(const Angle& _angle)
        {
            text.SetRotation(_angle);
        }
        INLINE Angle GetRotation() const
        {
            return Angle(text.GetRotation());
        }

        INLINE void SetTransform(const Transform& _transform)
        {
            SetPosition(_transform.position);
            SetRotation(_transform.rotation);
            SetScale(_transform.scale);
        }
        INLINE Transform GetTransform() const
        {
            return Transform(GetPosition(), GetRotation(), GetScale());
        }


        INLINE FRect GetLocalBounds() const
        {
            return text.GetLocalBounds();
        }
        INLINE FRect GetGlobalBounds() const
        {
            return text.GetGlobalBounds();
        }
        INLINE FVector2 FindCharacterPos(const std::size_t& _index) const
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

