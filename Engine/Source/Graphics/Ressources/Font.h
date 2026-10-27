#pragma once
#include "Utilities/Macro.h"

namespace Krampus
{

	/// <summary>
	/// Supported font file format extensions.
	/// </summary>
	enum class FontExtensionType
	{
		OTF,
		TTF
	};

	/// <summary>
	/// Represents a typographic font used to render text strings.
	/// </summary>
	class Font
	{
		sf::Font	font;

	public:
		/// <summary>
		/// Checks whether bilinear smoothing is enabled for font rasterization.
		/// </summary>
		/// <returns>True if smooth, false otherwise.</returns>
		inline Bool IsSmooth() const
		{
			return font.isSmooth();
		}

		/// <summary>
		/// Sets whether bilinear smoothing is enabled for font rasterization.
		/// </summary>
		/// <param name="_smooth">True to enable smoothing.</param>
		inline void SetSmooth(const Bool& _smooth)
		{
			return font.setSmooth(_smooth);
		}

		/// <summary>
		/// Default constructor for Font.
		/// </summary>
		Font() = default;

		/// <summary>
		/// Constructs and loads a Font from a file path.
		/// </summary>
		/// <param name="_path">File path to font.</param>
		Font(const String& _path) : font(_path.StdString()) {}

		/// <summary>
		/// Constructs a Font copying from an SFML sf::Font.
		/// </summary>
		/// <param name="_font">SFML font instance.</param>
		Font(const sf::Font& _font) : font(_font) {}

		/// <summary>
		/// Loads a font from a specified file path.
		/// </summary>
		/// <param name="_path">Path to the font file.</param>
		/// <returns>True if loaded successfully, false otherwise.</returns>
		inline Bool Open(const String& _path)
		{
			return font.openFromFile(_path.StdString());
		}

		/// <summary>
		/// Implicit conversion to const reference of the underlying sf::Font.
		/// </summary>
		/// <returns>Reference to internal sf::Font.</returns>
		operator const sf::Font&() const noexcept
		{
			return font;
		}
	};

}
