#pragma once
#include "Utilities/Math/Rectangle.h"
#include "Utilities/Color.h"

namespace Krampus
{
	class Image
	{
		sf::Image image;

	public:
        INLINE UVector2 GetSize() const
        {
            return UVector2(image.getSize());
        }
        INLINE void SetPixel(const UVector2& _coords, const Color& _color)
        {
            image.setPixel(_coords, _color);
        }
        INLINE Color GetPixel(const UVector2& _coords) const
        {
            return Krampus::Color(image.getPixel(_coords));
        }
        INLINE const std::uint8_t* GetPixelsPtr() const
        {
            return image.getPixelsPtr();
        }


		Image();
        explicit Image(const UVector2& _size, const Color& _color = Color::Black());
        Image(const UVector2& _size, const std::uint8_t* _pixels);
        explicit Image(const std::string& _path);
        Image(const void* _data, const std::size_t& _size);
        Image(const sf::Image& _image);

        void Resize(const UVector2& _size, const Color& _color = Color::Black());
        void ResizeFromPixels(const UVector2& _size, const std::uint8_t* _pixels);

        bool LoadFromFile(const std::string& _filename);
        bool LoadFromMemory(const void* _data, const std::size_t& _size);

        bool SaveToFile(const std::string& _filename) const;
        std::optional<std::vector<std::uint8_t>> SaveToMemory(const std::string_view& _format) const;

        void CreateMaskFromColor(const Color& _color, const std::uint8_t& _alpha = 0);

        bool Copy(const Image& _source, const UVector2& _dest,
            const IRect& _sourceRect = IRect(), const bool& _applyAlpha = false);

        void FlipHorizontally();
        void FlipVertically();

		INLINE operator sf::Image() const noexcept
		{
			return image;
		}
	};

}