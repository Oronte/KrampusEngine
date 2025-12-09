#pragma once
#include "Graphics/Ressources/Font.h"
#include "Utilities/Singleton.h"

#define M_FONT Krampus::FontManager::GetInstance()

namespace Krampus
{
	class Text;

	class FontManager : public Singleton<FontManager>
	{
		std::string fontPath = "Content/Fonts/";
		std::string defaultFontPath = "DefaultFont";
		std::string defaultFontExtension = "otf";
		Font defaultFont;

	public:
		FontManager();

	private:
		void LoadDefaultFont();
		void SetFontToDefault(Font& _font);
		std::string GetExtensionNameByType(const FontExtensionType& _fontType) const;

	public:
		void LoadFont(Font& _font, const std::string& _fullName);
		void LoadFont(Font& _font, const std::string& _fileName, const FontExtensionType& _fontType);
	};


}