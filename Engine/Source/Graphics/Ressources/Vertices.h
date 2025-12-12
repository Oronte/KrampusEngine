#pragma once
#include "Utilities/Macro.h"
#include "Utilities/Math/Rectangle.h"
#include "Utilities/Color.h"

namespace Krampus
{
    using Vertex = sf::Vertex;
    using PrimitiveType = sf::PrimitiveType;

	class Vertices
	{
		sf::VertexArray vertices;

    public:

        NO_DISCARD INLINE std::size_t GetVertexCount() const
        {
            const std::size_t& _size = vertices.getVertexCount();
            return _size;
        }
        NO_DISCARD INLINE PrimitiveType GetPrimitiveType() const
        {
            const PrimitiveType& _type = vertices.getPrimitiveType();
            return _type;
        }
        INLINE void SetPrimitiveType(const PrimitiveType& _type)
        {
            return vertices.setPrimitiveType(_type);
        }
        NO_DISCARD INLINE FRect GetBounds() const
        {
            return FRect(vertices.getBounds());
        }

        Vertices() = default;
        explicit Vertices(const PrimitiveType& _type, const std::size_t& _vertexCount = 0);

        void Clear();
        void Resize(const std::size_t& _vertexCount);
        void Append(const FVector2& _position, const Color& _color = Color::Magenta(), const FVector2& _textureCoords = FVector2());;
        void Append(const Vertex& _vertex);

        NO_DISCARD INLINE Vertex& operator[](const std::size_t& _index)
        {
            return vertices[_index];
        }
        NO_DISCARD INLINE const Vertex& operator[](const std::size_t& _index) const
        {
            return vertices[_index];
        }

        INLINE operator sf::VertexArray() const noexcept
        {
            return vertices;
        }
	};

}