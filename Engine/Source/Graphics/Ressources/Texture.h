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
		std::shared_ptr<sf::Texture> texture;

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


        Texture() = default;
        explicit Texture(std::shared_ptr<sf::Texture> _texture) noexcept
            : texture(std::move(_texture)) { }
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


        INLINE Texture& operator = (const Texture& _other) noexcept
        {
            this->texture = _other.texture;
            return *this;
        }

        INLINE Texture& operator = (Texture&& _other) noexcept
        {
            this->texture = std::move(_other.texture);
            return *this;
        }

        INLINE operator sf::Texture() const noexcept
        {
            if (!texture) return sf::Texture();
            return *texture.get();
        }

        INLINE void SetSharedTexture(std::shared_ptr<sf::Texture> _texture) noexcept
        {
            texture = std::move(_texture);
        }
	};

}