#pragma once
#include "Graphics/Ressources/Font.h"
#include "Utilities/System/Singleton.h"

#define M_FONT Krampus::FontManager::GetInstance()

namespace Krampus
{
	class Text;

	/// <summary>
	/// Singleton manager responsible for loading, caching, and serving typographic Font assets.
	/// </summary>
	class FontManager : public Singleton<FontManager>
	{
		String	fontPath				= CONTENT_PATH + "Fonts/";
		String	defaultFontPath			= "_internal/DefaultFont";
		String	defaultFontExtension	= "otf";
		Font	defaultFont;

	public:
		/// <summary>
		/// Constructs the FontManager and loads the default fallback font.
		/// </summary>
		FontManager();

	private:
		void LoadDefaultFont();
		void SetFontToDefault(Font& _font);
		String GetExtensionNameByType(const FontExtensionType& _fontType) const;

	public:
		/// <summary>
		/// Loads a font from a full file path into the given Font object.
		/// </summary>
		/// <param name="_font">Target Font to populate.</param>
		/// <param name="_fullName">Full file name with extension.</param>
		void LoadFont(Font& _font, const String& _fullName);

		/// <summary>
		/// Loads a font from Content/Fonts/ using a base name and font extension enum.
		/// </summary>
		/// <param name="_font">Target Font to populate.</param>
		/// <param name="_fileName">Font file name without extension.</param>
		/// <param name="_fontType">FontExtensionType (OTF, TTF).</param>
		void LoadFont(Font& _font, const String& _fileName, const FontExtensionType& _fontType);
	};

}
