#include "Vertices.h"

Krampus::Vertices::Vertices(const PrimitiveType& _type, const std::size_t& _vertexCount)
{
	vertices = sf::VertexArray(_type, _vertexCount);
}

void Krampus::Vertices::Clear()
{
	vertices.clear();
}

void Krampus::Vertices::Resize(const std::size_t& _vertexCount)
{
	vertices.resize(_vertexCount);
}

void Krampus::Vertices::Append(const FVector2& _position, const Color& _color, const FVector2& _textureCoords)
{
	Vertex _vertex;
	_vertex.position = _position;
	_vertex.color = _color;
	_vertex.texCoords = _textureCoords;
	Append(_vertex);
}

void Krampus::Vertices::Append(const Vertex& _vertex)
{
	vertices.append(_vertex);
}