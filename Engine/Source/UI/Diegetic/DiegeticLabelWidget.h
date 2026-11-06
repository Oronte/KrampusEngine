#pragma once
#include "DiegeticWidget.h"
#include "Components/LabelComponent.h"

namespace Krampus
{

	/// <summary>
	/// In-world diegetic label actor that displays and formats text in the scene.
	/// </summary>
	class DiegeticLabelWidget : public DiegeticWidget
	{
	protected:
		LabelComponent*		label	= nullptr;

	public:
        /// <summary>Sets displayed string.</summary>
        /// <param name="_string">Text string.</param>
        void SetString(const String& _string)
        {
            label->SetString(_string);
        }

        /// <summary>Gets displayed string.</summary>
        /// <returns>Text string.</returns>
        String GetString() const
        {
            return label->GetString();
        }

        /// <summary>Sets font resource.</summary>
        /// <param name="_fileName">Font resource name.</param>
        /// <param name="_type">Font extension type.</param>
        void SetFont(const String& _fileName, const FontExtensionType& _type)
        {
            label->SetFont(_fileName, _type);
        }

        /// <summary>Gets font resource.</summary>
        /// <returns>Const reference to Font.</returns>
        const Font& GetFont() const
        {
            return label->GetFont();
        }

        /// <summary>Sets character font size.</summary>
        /// <param name="_size">Size in pixels.</param>
        void SetCharacterSize(const UInt& _size)
        {
            label->SetCharacterSize(_size);
        }

        /// <summary>Gets character font size.</summary>
        /// <returns>Size in pixels.</returns>
        UInt GetCharacterSize() const
        {
            return label->GetCharacterSize();
        }

        /// <summary>Sets text fill color.</summary>
        /// <param name="_color">Fill color.</param>
        void SetFillColor(const Color& _color)
        {
            label->SetFillColor(_color);
        }

        /// <summary>Gets text fill color.</summary>
        /// <returns>Fill color.</returns>
        Color GetFillColor() const
        {
            return label->GetFillColor();
        }

        /// <summary>Sets outline border color.</summary>
        /// <param name="_color">Outline color.</param>
        void SetOutlineColor(const Color& _color)
        {
            label->SetOutlineColor(_color);
        }

        /// <summary>Gets outline border color.</summary>
        /// <returns>Outline color.</returns>
        Color GetOutlineColor() const
        {
            return label->GetOutlineColor();
        }

        /// <summary>Sets outline thickness in pixels.</summary>
        /// <param name="_thickness">Outline thickness.</param>
        void SetOutlineThickness(const Float& _thickness)
        {
            label->SetOutlineThickness(_thickness);
        }

        /// <summary>Gets outline thickness in pixels.</summary>
        /// <returns>Outline thickness.</returns>
        Float GetOutlineThickness() const
        {
            return label->GetOutlineThickness();
        }

        /// <summary>Sets style flags (Bold, Italic, etc.).</summary>
        /// <param name="_style">Style bitmask flags.</param>
        void SetStyle(const UInt& _style)
        {
            label->SetStyle(_style);
        }

        /// <summary>Gets style flags.</summary>
        /// <returns>Style flags.</returns>
        UInt GetStyle() const
        {
            return label->GetStyle();
        }

        /// <summary>Sets character spacing factor.</summary>
        /// <param name="_spacing">Letter spacing.</param>
        void SetLetterSpacing(const Float& _spacing)
        {
            label->SetLetterSpacing(_spacing);
        }

        /// <summary>Gets character spacing factor.</summary>
        /// <returns>Letter spacing.</returns>
        Float GetLetterSpacing() const
        {
            return label->GetLetterSpacing();
        }

        /// <summary>Sets line spacing factor.</summary>
        /// <param name="_spacing">Line spacing.</param>
        void SetLineSpacing(const Float& _spacing)
        {
            label->SetLineSpacing(_spacing);
        }

        /// <summary>Gets line spacing factor.</summary>
        /// <returns>Line spacing.</returns>
        Float GetLineSpacing() const
        {
            return label->GetLineSpacing();
        }

        /// <summary>Sets local origin pivot coordinates.</summary>
        /// <param name="_origin">Origin vector.</param>
        void SetOrigin(const FVector2& _origin)
        {
            label->SetOrigin(_origin);
        }

        /// <summary>Gets local origin pivot coordinates.</summary>
        /// <returns>Origin vector.</returns>
        FVector2 GetOrigin() const
        {
            return label->GetOrigin();
        }

        /// <summary>Gets local untransformed bounding box.</summary>
        /// <returns>Local bounds FRect.</returns>
        FRect GetLocalBounds() const
        {
            return label->GetLocalBounds();
        }

        /// <summary>Gets transformed global bounding box.</summary>
        /// <returns>Global bounds FRect.</returns>
        FRect GetGlobalBounds() const
        {
            return label->GetGlobalBounds();
        }

        /// <summary>Finds glyph position at index.</summary>
        /// <param name="_index">Glyph index.</param>
        /// <returns>Position vector.</returns>
        FVector2 FindCharacterPos(const Int& _index) const
        {
            return label->FindCharacterPos(_index);
        }

        /// <summary>Constructs a DiegeticLabelWidget with initial text data configuration.</summary>
        /// <param name="_level">Owning Level.</param>
        /// <param name="_data">Text data settings.</param>
		/// <summary>A text "Label": what the editor spawns (change its LabelComponent in the Inspector).</summary>
		explicit DiegeticLabelWidget(Level* _level);

		DiegeticLabelWidget(Level* _level, const TextData& _data);

        /// <summary>Returns a string representation of the diegetic label.</summary>
        /// <returns>String representation.</returns>
		std::string ToString() const override;
	};

}
