#pragma once
#include "DrawnComponent.h"
#include "Graphics/Resources/Text.h"
#include "Utilities/System/Debuggable.h"

namespace Krampus
{

	/// <summary>
	/// A text that follows its owner's transform. Shared by LabelComponent (world) and LabelWidgetComponent (HUD).
	/// </summary>
	template <typename Base>
	class BasicLabel : public DrawnComponent<Base>, public IDebuggable
	{
		Event<FVector2>::ListenerHandle onOwnerMoveHandle;
		Event<Angle>::ListenerHandle	onOwnerRotateHandle;
		Event<FVector2>::ListenerHandle onOwnerScaleHandle;

	protected:
		Text text;

		BasicLabel(typename Base::OwnerType* _owner, const TextData& _data);

	public:
		void SetString(const String& _string)	{ text.SetString(_string); }
		[[nodiscard]] String GetString() const	{ return text.GetString(); }

		void SetFont(const String& _fileName, const FontExtensionType& _type)	{ text.SetFont(_fileName, _type); }
		[[nodiscard]] const String& GetFontFileName() const						{ return text.GetFontFileName(); }
		[[nodiscard]] FontExtensionType GetFontType() const						{ return text.GetFontType(); }
		[[nodiscard]] const Font& GetFont() const								{ return text.GetFont(); }

		void SetCharacterSize(const UInt& _size)	{ text.SetCharacterSize(_size); }
		[[nodiscard]] UInt GetCharacterSize() const	{ return text.GetCharacterSize(); }

		void SetFillColor(const Color& _color)		{ text.SetFillColor(_color); }
		[[nodiscard]] Color GetFillColor() const	{ return text.GetFillColor(); }
		void SetOutlineColor(const Color& _color)	{ text.SetOutlineColor(_color); }
		[[nodiscard]] Color GetOutlineColor() const	{ return text.GetOutlineColor(); }
		void SetOutlineThickness(const Float& _thickness)	{ text.SetOutlineThickness(_thickness); }
		[[nodiscard]] Float GetOutlineThickness() const		{ return text.GetOutlineThickness(); }

		/// <summary>sf::Text::Style flags (bold, italic, underlined...).</summary>
		void SetStyle(const UInt& _style)		{ text.SetStyle(_style); }
		[[nodiscard]] UInt GetStyle() const		{ return text.GetStyle(); }

		void SetLetterSpacing(const Float& _spacing)	{ text.SetLetterSpacing(_spacing); }
		[[nodiscard]] Float GetLetterSpacing() const	{ return text.GetLetterSpacing(); }
		void SetLineSpacing(const Float& _spacing)		{ text.SetLineSpacing(_spacing); }
		[[nodiscard]] Float GetLineSpacing() const		{ return text.GetLineSpacing(); }

		void SetOrigin(const FVector2& _origin)		{ text.SetOrigin(_origin); }
		[[nodiscard]] FVector2 GetOrigin() const	{ return text.GetOrigin(); }

		[[nodiscard]] FRect GetLocalBounds() const	{ return text.GetLocalBounds(); }
		[[nodiscard]] FRect GetGlobalBounds() const	{ return text.GetGlobalBounds(); }
		/// <summary>Position of the character at _index, in the owner's space.</summary>
		[[nodiscard]] FVector2 FindCharacterPos(const Int& _index) const { return text.FindCharacterPos(_index); }

		std::string ToString() const override;

	private:
		void Draw(Window& _window) override;
		void DrawDebug() override;
	};

	extern template class BasicLabel<Component>;
	extern template class BasicLabel<WidgetComponent>;

}
