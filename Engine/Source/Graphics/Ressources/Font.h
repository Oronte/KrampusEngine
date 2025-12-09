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
		sf::Font font;

	public:
		INLINE bool IsSmooth() const
		{
			return font.isSmooth();
		}
		INLINE void SetSmooth(const bool& _smooth)
		{
			return font.setSmooth(_smooth);
		}

		Font() = default;
		Font(const std::string& _path) : font(_path) {}
		Font(const sf::Font& _font) : font(_font) {}

		INLINE bool Open(const std::string& _path)
		{
			return font.openFromFile(_path);
		}

		operator const sf::Font&() const noexcept
		{
			return font;
		}
	};

}