#pragma once
#include "Graphics/Ressources/Font.h"
#include "Utilities/System/Singleton.h"

#define M_FONT Krampus::FontManager::GetInstance()

namespace Krampus
{
	class Text;

	class FontManager : public Singleton<FontManager>
	{
#ifdef SHIP
		String	fontPath				= "../../Content/Fonts/";
#else
		String	fontPath				= CONTENT_PATH + "Fonts/";
#endif
		String	defaultFontPath			= "_internal/DefaultFont";
		String	defaultFontExtension	= "otf";
		Font	defaultFont;

	public:
		FontManager();

	private:
		void LoadDefaultFont();
		void SetFontToDefault(Font& _font);
		String GetExtensionNameByType(const FontExtensionType& _fontType) const;

	public:
		void LoadFont(Font& _font, const String& _fullName);
		void LoadFont(Font& _font, const String& _fileName, const FontExtensionType& _fontType);
	};


}