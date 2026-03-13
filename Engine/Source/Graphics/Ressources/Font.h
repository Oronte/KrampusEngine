#pragma once
#include "Utilities/Macro.h"


namespace Krampus
{

	enum class FontExtensionType
	{
		OTF,
		TTF
	};

	class Font
	{
		sf::Font	font;

	public:
		inline Bool IsSmooth() const
		{
			return font.isSmooth();
		}
		inline void SetSmooth(const Bool& _smooth)
		{
			return font.setSmooth(_smooth);
		}

		Font() = default;
		Font(const String& _path) : font(_path.StdString()) {}
		Font(const sf::Font& _font) : font(_font) {}

		inline Bool Open(const String& _path)
		{
			return font.openFromFile(_path.StdString());
		}

		operator const sf::Font&() const noexcept
		{
			return font;
		}
	};

}