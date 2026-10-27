#pragma once
#include "Utilities/Math/Rectangle.h"
#include "Utilities/Debug/Color.h"

namespace Krampus
{
	/// <summary>
	/// Represents an image buffer stored in system RAM, providing pixel-level manipulation.
	/// </summary>
	class Image
	{
		sf::Image   image;

	public:
		/// <summary>
		/// Gets the width and height of the image in pixels.
		/// </summary>
		/// <returns>Image dimensions as UVector2.</returns>
        inline UVector2 GetSize() const
        {
            return UVector2(image.getSize());
        }

		/// <summary>
		/// Sets the color of the pixel at the specified coordinate.
		/// </summary>
		/// <param name="_coords">Pixel coordinate (X, Y).</param>
		/// <param name="_color">Color to assign.</param>
        inline void SetPixel(const UVector2& _coords, const Color& _color)
        {
            image.setPixel(_coords, _color);
        }

		/// <summary>
		/// Retrieves the color of the pixel at the specified coordinate.
		/// </summary>
		/// <param name="_coords">Pixel coordinate (X, Y).</param>
		/// <returns>Pixel color.</returns>
        inline Color GetPixel(const UVector2& _coords) const
        {
            return Krampus::Color(image.getPixel(_coords));
        }

		/// <summary>
		/// Gets a read-only raw pointer to the RGBA byte array in memory (4 bytes per pixel).
		/// </summary>
		/// <returns>Pointer to RGBA pixel bytes.</returns>
        inline const std::uint8_t* GetPixelsPtr() const
        {
            return image.getPixelsPtr();
        }

		/// <summary>
		/// Default constructor creating an empty image.
		/// </summary>
		Image();

		/// <summary>
		/// Constructs an image with specified dimensions initialized to a uniform color.
		/// </summary>
		/// <param name="_size">Dimensions in pixels.</param>
		/// <param name="_color">Fill color.</param>
        explicit Image(const UVector2& _size, const Color& _color = Color::Black());

		/// <summary>
		/// Constructs an image with specified dimensions initialized from raw RGBA pixel data.
		/// </summary>
		/// <param name="_size">Dimensions in pixels.</param>
		/// <param name="_pixels">Pointer to 32-bit RGBA pixels.</param>
        Image(const UVector2& _size, const std::uint8_t* _pixels);

		/// <summary>
		/// Constructs an image by loading from a file path.
		/// </summary>
		/// <param name="_path">Path to the image file.</param>
        Image(const String& _path);

		/// <summary>
		/// Constructs an image by decoding image data from a memory buffer.
		/// </summary>
		/// <param name="_data">Pointer to memory buffer.</param>
		/// <param name="_size">Size of data in bytes.</param>
        Image(const void* _data, const std::size_t& _size);

		/// <summary>
		/// Constructs an Image by copying from an SFML sf::Image.
		/// </summary>
		/// <param name="_image">Source SFML image.</param>
        Image(const sf::Image& _image);

		/// <summary>
		/// Resizes the image and fills with a uniform color.
		/// </summary>
		/// <param name="_size">New dimensions.</param>
		/// <param name="_color">Fill color.</param>
        void Resize(const UVector2& _size, const Color& _color = Color::Black());

		/// <summary>
		/// Resizes the image and populates from raw RGBA pixels.
		/// </summary>
		/// <param name="_size">New dimensions.</param>
		/// <param name="_pixels">Pointer to RGBA pixels.</param>
        void ResizeFromPixels(const UVector2& _size, const std::uint8_t* _pixels);

		/// <summary>
		/// Loads the image from a file on disk.
		/// </summary>
		/// <param name="_filename">File path.</param>
		/// <returns>True if loaded successfully, false otherwise.</returns>
        Bool LoadFromFile(const String& _filename);

		/// <summary>
		/// Loads the image from encoded image bytes in memory.
		/// </summary>
		/// <param name="_data">Pointer to byte buffer.</param>
		/// <param name="_size">Buffer size in bytes.</param>
		/// <returns>True if decoded successfully, false otherwise.</returns>
        Bool LoadFromMemory(const void* _data, const std::size_t& _size);

		/// <summary>
		/// Saves the image to a file on disk.
		/// </summary>
		/// <param name="_filename">Target file path.</param>
		/// <returns>True if saved successfully, false otherwise.</returns>
        Bool SaveToFile(const String& _filename) const;

		/// <summary>
		/// Encodes and saves the image to a memory byte vector in the given format (e.g. "png", "bmp").
		/// </summary>
		/// <param name="_format">Format string identifier.</param>
		/// <returns>Optional vector containing encoded bytes.</returns>
        std::optional<std::vector<std::uint8_t>> SaveToMemory(const std::string_view& _format) const;

		/// <summary>
		/// Converts all pixels matching a specific color to have an explicit alpha channel.
		/// </summary>
		/// <param name="_color">Key color to match.</param>
		/// <param name="_alpha">New alpha value to assign.</param>
        void CreateMaskFromColor(const Color& _color, const std::uint8_t& _alpha = 0);

		/// <summary>
		/// Copies a sub-region of another image into this image at a target offset.
		/// </summary>
		/// <param name="_source">Source image to copy from.</param>
		/// <param name="_dest">Destination coordinates in this image.</param>
		/// <param name="_sourceRect">Source sub-rectangle to copy (0 for entire source).</param>
		/// <param name="_applyAlpha">Whether to alpha-blend pixels during copy.</param>
		/// <returns>True if copy succeeded, false on error.</returns>
        Bool Copy(const Image& _source, const UVector2& _dest,
            const IRect& _sourceRect = IRect(), const Bool& _applyAlpha = false);

		/// <summary>
		/// Flips the image horizontally across its vertical axis.
		/// </summary>
        void FlipHorizontally();

		/// <summary>
		/// Flips the image vertically across its horizontal axis.
		/// </summary>
        void FlipVertically();

		/// <summary>
		/// Implicit conversion to sf::Image.
		/// </summary>
        inline operator sf::Image() const noexcept
		{
			return image;
		}
	};

}
