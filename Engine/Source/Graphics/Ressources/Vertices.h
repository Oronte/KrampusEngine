#pragma once
#include "Utilities/Macro.h"
#include "Utilities/Math/Rectangle.h"
#include "Utilities/Debug/Color.h"

namespace Krampus
{
    /// <summary>
    /// Type alias for SFML 2D vertex containing 2D position, color, and texture coordinates.
    /// </summary>
    using Vertex = sf::Vertex;

    /// <summary>
    /// Type alias for primitive rendering topology (Points, Lines, Triangles, etc.).
    /// </summary>
    using PrimitiveType = sf::PrimitiveType;

	/// <summary>
	/// Manages an array of 2D vertices defining geometric primitives (triangles, quads, lines).
	/// </summary>
	class Vertices
	{
		sf::VertexArray     vertices;

    public:

		/// <summary>
		/// Gets the number of vertices in the array.
		/// </summary>
		/// <returns>Vertex count.</returns>
        NO_DISCARD inline ULongLong GetVertexCount() const
        {
            const std::size_t& _size = vertices.getVertexCount();
            return _size;
        }

		/// <summary>
		/// Gets the geometric primitive rendering topology.
		/// </summary>
		/// <returns>PrimitiveType enum value.</returns>
        NO_DISCARD inline PrimitiveType GetPrimitiveType() const
        {
            const PrimitiveType& _type = vertices.getPrimitiveType();
            return _type;
        }

		/// <summary>
		/// Sets the primitive rendering topology type.
		/// </summary>
		/// <param name="_type">PrimitiveType enum value.</param>
        inline void SetPrimitiveType(const PrimitiveType& _type)
        {
            return vertices.setPrimitiveType(_type);
        }

		/// <summary>
		/// Computes the 2D bounding rectangle enclosing all vertices in the array.
		/// </summary>
		/// <returns>Axis-aligned bounding box as FRect.</returns>
        NO_DISCARD inline FRect GetBounds() const
        {
            return FRect(vertices.getBounds());
        }

		/// <summary>
		/// Default constructor creating an empty vertex array with Points topology.
		/// </summary>
        Vertices() = default;

		/// <summary>
		/// Constructs a vertex array with specified primitive topology and initial vertex capacity.
		/// </summary>
		/// <param name="_type">Primitive topology.</param>
		/// <param name="_vertexCount">Initial vertex count.</param>
        Vertices(const PrimitiveType& _type, const ULongLong& _vertexCount = 0);

		/// <summary>
		/// Clears all vertices from the array.
		/// </summary>
        void Clear();

		/// <summary>
		/// Resizes the vertex array to hold a specified number of vertices.
		/// </summary>
		/// <param name="_vertexCount">New vertex count.</param>
        void Resize(const ULongLong& _vertexCount);

		/// <summary>
		/// Appends a new vertex with position, color, and UV texture coordinates.
		/// </summary>
		/// <param name="_position">2D position.</param>
		/// <param name="_color">Vertex color.</param>
		/// <param name="_textureCoords">Texture UV coordinates.</param>
        void Append(const FVector2& _position, const Color& _color = Color::Magenta(), const FVector2& _textureCoords = FVector2());

		/// <summary>
		/// Appends an existing Vertex structure.
		/// </summary>
		/// <param name="_vertex">Vertex to append.</param>
        void Append(const Vertex& _vertex);

		/// <summary>
		/// Subscript operator for mutable access to vertex by index.
		/// </summary>
		/// <param name="_index">Zero-based index.</param>
		/// <returns>Reference to Vertex.</returns>
        NO_DISCARD inline Vertex& operator[](const std::size_t& _index)
        {
            return vertices[_index];
        }

		/// <summary>
		/// Subscript operator for read-only access to vertex by index.
		/// </summary>
		/// <param name="_index">Zero-based index.</param>
		/// <returns>Const reference to Vertex.</returns>
        NO_DISCARD inline const Vertex& operator[](const std::size_t& _index) const
        {
            return vertices[_index];
        }

		/// <summary>The SFML array itself, without a copy.</summary>
        inline const sf::VertexArray& Get() const noexcept
        {
            return vertices;
        }

		/// <summary>
		/// Implicit conversion operator to SFML sf::VertexArray.
		/// </summary>
        inline operator sf::VertexArray() const noexcept
        {
            return vertices;
        }
	};

}
