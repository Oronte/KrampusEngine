#pragma once
#include "Image.h"
#include "Utilities/Logger.h"

namespace Krampus
{

    enum class TextureExtensionType
    {
        PNG,
        JPG,
        GIF
    };

	class Texture
	{
		std::unique_ptr<sf::Texture> texture;

	public:
        INLINE sf::Texture* Get() const noexcept
        {
            return texture.get();
        }
        INLINE UVector2 GetSize() const
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return UVector2();
            }
            return UVector2(texture->getSize());
        }
        INLINE bool IsRepeated() const
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return false;
            }
            return texture->isRepeated();
        }
        INLINE void SetRepeated(bool _repeated)
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return;
            }
            texture->setRepeated(_repeated);
        }
        INLINE bool IsSmooth() const
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return false;
            }
            return texture->isSmooth();
        }
        INLINE void SetSmooth(bool _smooth)
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return;
            }
            texture->setSmooth(_smooth);
        }
        INLINE bool IsSrgb() const
        {
            if (!texture)
            {
                LOG(VerbosityType::Error, "You try to use a non initialize texture (potential fake values)");
                return false;
            }
            return texture->isSrgb();
        }


        Texture();
        explicit Texture(const std::string& _path, const bool& _rgb = false);
        Texture(const std::string& _path, const bool& _rgb, const IRect& _area);
        Texture(const void* data, std::size_t _size, const bool& _rgb = false);
        Texture(const void* data, std::size_t _size, const bool& _rgb, const IRect& _area);
        explicit Texture(const Image& _image, const bool& _rgb = false);
        Texture(const Image& _image, const bool& _rgb, const IRect& _area);
        explicit Texture(const UVector2& _size, const bool& _rgb = false);
        explicit Texture(const sf::Texture& _texture);

        bool Resize(const UVector2& _size, const bool& _sRgb = false);

        bool LoadFromFile(const std::string& _filename, const bool& _sRgb = false, const IRect& _area = IRect());
        bool LoadFromMemory(const void* _data, const std::size_t& _size, const bool& _rgb = false, const IRect& _area = IRect());
        bool LoadFromImage(const Image& _image, const bool& _rgb = false, const IRect& _area = IRect());

        void Bind(const Texture* _texture, const bool& _isNoralize = true);

        Image CopyToImage() const;

        bool GenerateMipmap();

        INLINE static unsigned int GetMaximumSize()
        {
            return sf::Texture::getMaximumSize();
        }


        INLINE void operator = (const Texture& _other) noexcept
        {
            if (!_other.texture)
            {
                LOG(VerbosityType::Error, "You try to set the value of texture with a nullptr texture");
                return;
            }
            texture = std::make_unique<sf::Texture>(*(_other.texture.get()));
        }
        INLINE operator sf::Texture() const noexcept
        {
            return *(texture.get());
        }
	};

}