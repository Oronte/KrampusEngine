#pragma once
#include "Utilities/System/Language.h"
#include "Text.h"
#include "Core/Engine.h"

namespace Krampus
{

	struct TranslateTextData
	{
		std::map<Language, std::string> texts;        
		unsigned int characterSize = 30u;
		std::string fontFileName = "DefaultFont";
		FontExtensionType extentionType = FontExtensionType::OTF;

		operator TextData()
		{
			TextData _data;
			_data.characterSize = characterSize;
			_data.extentionType = extentionType;
			_data.fontFileName = fontFileName;
			_data.text = texts[ENGINE.GetLanguage()];
			return _data;
		}
	};

	class TranslateText : public Text
	{
		std::map<Language, std::string> strings;

	public:
		TranslateText(const TextData& _data)
			: Text(_data)
		{
			strings[ENGINE.GetLanguage()] = _data.text;
		}
		TranslateText(const TranslateTextData& _data)
			: Text(_data)
		{
			strings = _data.texts;
		}

		INLINE virtual void SetString(const std::string& _string) override
		{
			strings[ENGINE.GetLanguage()] = _string;
			Text::SetString(_string);
		}

	private:
		INLINE void SetString(const Language& _language)
		{
			Text::SetString(strings[_language]);
		}
	};

}