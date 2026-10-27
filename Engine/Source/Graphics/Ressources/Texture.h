#pragma once
#include "Image.h"
#include "Utilities/Debug/Logger.h"

namespace Krampus
{

    /// <summary>
    /// Supported image format extensions for 2D textures.
    /// </summary>
    enum class TextureExtensionType
    {
        PNG,
        JPG,
        GIF,
        BMP,
        TGA,
        PSD,
        HDR,
        PIC,
        PNM
    };

	/// <summary>
	/// Manages 2D GPU texture memory, sampling modes, and image data binding.
	/// </summary>
	class Texture
	{
		std::shared_ptr<sf::Texture>    texture;

	public:
		/// <summary>
		/// Gets a raw pointer to the underlying SFML texture.
		/// </summary>
		/// <returns>Pointer to sf::Texture.</returns>
        inline sf::Texture* Get() const noexcept
        {
            return texture.get();
        }

		/// <summary>
		/// Gets the dimensions of the texture in pixels.
		/// </summary>
		/// <returns>Dimensions as UVector2.</returns>
        inline UVector2 GetSize() const
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return UVector2();
            }
            return UVector2(texture->getSize());
        }

		/// <summary>
		/// Checks whether texture wrapping/repeating is enabled.
		/// </summary>
		/// <returns>True if repeating, false otherwise.</returns>
        inline Bool IsRepeated() const
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return false;
            }
            return texture->isRepeated();
        }

		/// <summary>
		/// Enables or disables texture repeating/tiling.
		/// </summary>
		/// <param name="_repeated">True to repeat texture, false to clamp.</param>
        inline void SetRepeated(const Bool& _repeated)
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return;
            }
            texture->setRepeated(_repeated);
        }

		/// <summary>
		/// Checks whether bilinear filtering (smoothing) is enabled.
		/// </summary>
		/// <returns>True if smooth filtering is on.</returns>
        inline Bool IsSmooth() const
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return false;
            }
            return texture->isSmooth();
        }

		/// <summary>
		/// Enables or disables bilinear filtering (smoothing).
		/// </summary>
		/// <param name="_smooth">True to smooth, false for point sampling.</param>
        inline void SetSmooth(const Bool& _smooth)
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return;
            }
            texture->setSmooth(_smooth);
        }

		/// <summary>
		/// Checks whether the texture uses sRGB color space conversion.
		/// </summary>
		/// <returns>True if sRGB, false otherwise.</returns>
        inline Bool IsSrgb() const
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return false;
            }
            return texture->isSrgb();
        }

		/// <summary>
		/// Default constructor for Texture.
		/// </summary>
        Texture() = default;

		/// <summary>
		/// Constructs a Texture sharing an existing SFML texture pointer.
		/// </summary>
		/// <param name="_texture">Shared pointer to sf::Texture.</param>
        explicit Texture(std::shared_ptr<sf::Texture> _texture) noexcept
            : texture(std::move(_texture)) { }

		/// <summary>
		/// Constructs and loads a Texture from a file path.
		/// </summary>
		/// <param name="_path">File path.</param>
		/// <param name="_rgb">Whether sRGB color space is enabled.</param>
        Texture(const String& _path, const Bool& _rgb = false);

		/// <summary>
		/// Constructs and loads a Texture from a sub-rectangle of a file.
		/// </summary>
		/// <param name="_path">File path.</param>
		/// <param name="_rgb">sRGB enabled flag.</param>
		/// <param name="_area">Sub-rectangle area to load.</param>
        Texture(const String& _path, const Bool& _rgb, const IRect& _area);

		/// <summary>
		/// Constructs a Texture from encoded image bytes in memory.
		/// </summary>
		/// <param name="data">Pointer to memory buffer.</param>
		/// <param name="_size">Size of buffer in bytes.</param>
		/// <param name="_rgb">sRGB flag.</param>
        Texture(const void* data, const ULongLong& _size, const Bool& _rgb = false);

		/// <summary>
		/// Constructs a Texture from encoded image bytes in memory with a sub-rectangle.
		/// </summary>
		/// <param name="data">Pointer to memory buffer.</param>
		/// <param name="_size">Size of buffer in bytes.</param>
		/// <param name="_rgb">sRGB flag.</param>
		/// <param name="_area">Sub-rectangle area.</param>
        Texture(const void* data, const ULongLong& _size, const Bool& _rgb, const IRect& _area);

		/// <summary>
		/// Constructs a Texture copied from an Image.
		/// </summary>
		/// <param name="_image">Source Image.</param>
		/// <param name="_rgb">sRGB flag.</param>
        explicit Texture(const Image& _image, const Bool& _rgb = false);

		/// <summary>
		/// Constructs a Texture copied from a sub-area of an Image.
		/// </summary>
		/// <param name="_image">Source Image.</param>
		/// <param name="_rgb">sRGB flag.</param>
		/// <param name="_area">Sub-rectangle to copy.</param>
        Texture(const Image& _image, const Bool& _rgb, const IRect& _area);

		/// <summary>
		/// Constructs an empty Texture with the specified dimensions.
		/// </summary>
		/// <param name="_size">Dimensions in pixels.</param>
		/// <param name="_rgb">sRGB flag.</param>
        explicit Texture(const UVector2& _size, const Bool& _rgb = false);

		/// <summary>
		/// Constructs a Texture by copying an SFML sf::Texture.
		/// </summary>
		/// <param name="_texture">Source SFML texture.</param>
        explicit Texture(const sf::Texture& _texture);

		/// <summary>
		/// Resizes the texture to new dimensions on the GPU.
		/// </summary>
		/// <param name="_size">New dimensions in pixels.</param>
		/// <param name="_sRgb">Whether to enable sRGB color space.</param>
		/// <returns>True if resized successfully.</returns>
        Bool Resize(const UVector2& _size, const Bool& _sRgb = false);

		/// <summary>
		/// Loads the texture from a file on disk.
		/// </summary>
		/// <param name="_filename">File path.</param>
		/// <param name="_sRgb">sRGB flag.</param>
		/// <param name="_area">Sub-rectangle to load (empty for entire image).</param>
		/// <returns>True if loaded successfully.</returns>
        Bool LoadFromFile(const String& _filename, const Bool& _sRgb = false, const IRect& _area = IRect());

		/// <summary>
		/// Loads the texture from encoded image data in memory.
		/// </summary>
		/// <param name="_data">Pointer to memory buffer.</param>
		/// <param name="_size">Buffer size in bytes.</param>
		/// <param name="_rgb">sRGB flag.</param>
		/// <param name="_area">Sub-rectangle area.</param>
		/// <returns>True if loaded successfully.</returns>
        Bool LoadFromMemory(const void* _data, const std::size_t& _size, const Bool& _rgb = false, const IRect& _area = IRect());

		/// <summary>
		/// Loads the texture from a RAM Image buffer.
		/// </summary>
		/// <param name="_image">Source Image.</param>
		/// <param name="_rgb">sRGB flag.</param>
		/// <param name="_area">Sub-rectangle area.</param>
		/// <returns>True if loaded successfully.</returns>
        Bool LoadFromImage(const Image& _image, const Bool& _rgb = false, const IRect& _area = IRect());

		/// <summary>
		/// Binds a texture for OpenGL rendering.
		/// </summary>
		/// <param name="_texture">Texture to bind.</param>
		/// <param name="_isNoralize">Whether to use normalized texture coordinates.</param>
        void Bind(const Texture* _texture, const Bool& _isNoralize = true);

		/// <summary>
		/// Copies the texture pixels from GPU memory back to a system RAM Image.
		/// </summary>
		/// <returns>Image containing texture pixels.</returns>
        Image CopyToImage() const;

		/// <summary>
		/// Generates mipmap hierarchy for the texture on the GPU.
		/// </summary>
		/// <returns>True if mipmaps were generated.</returns>
        Bool GenerateMipmap();

		/// <summary>
		/// Gets the maximum texture dimension supported by the GPU.
		/// </summary>
		/// <returns>Maximum size in pixels.</returns>
        inline static UInt GetMaximumSize()
        {
            return sf::Texture::getMaximumSize();
        }

		/// <summary>
		/// Copy assignment operator.
		/// </summary>
		/// <param name="_other">Source texture.</param>
		/// <returns>Reference to this texture.</returns>
        inline Texture& operator = (const Texture& _other) noexcept
        {
            this->texture = _other.texture;
            return *this;
        }

		/// <summary>
		/// Move assignment operator.
		/// </summary>
		/// <param name="_other">Source texture to move.</param>
		/// <returns>Reference to this texture.</returns>
        inline Texture& operator = (Texture&& _other) noexcept
        {
            this->texture = std::move(_other.texture);
            return *this;
        }

		/// <summary>
		/// Implicit conversion operator to sf::Texture.
		/// </summary>
        inline operator sf::Texture() const noexcept
        {
            if (!texture) return sf::Texture();
            return *texture.get();
        }

		/// <summary>
		/// Sets the internal shared pointer managing the sf::Texture.
		/// </summary>
		/// <param name="_texture">Shared pointer to assign.</param>
        inline void SetSharedTexture(std::shared_ptr<sf::Texture> _texture) noexcept
        {
            texture = std::move(_texture);
        }
	};

}
