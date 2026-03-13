#include "Shader.h"

Krampus::Shader::Shader(const String& _vertexShaderFileName, const String& _fragmentShaderFileName)
{
	shader = std::make_unique<sf::Shader>(std::filesystem::path(_vertexShaderFileName.StdString()),
		std::filesystem::path(_fragmentShaderFileName.StdString()));
}

Krampus::Shader::Shader(const String& _vertexShaderFileName, const String& _geometryShaderFileName, const String& _fragmentShaderFileName)
{
	shader = std::make_unique<sf::Shader>(std::filesystem::path(_vertexShaderFileName.StdString()),
		std::filesystem::path(_geometryShaderFileName.StdString()),
		std::filesystem::path(_fragmentShaderFileName.StdString()));
}

Krampus::Shader::Shader(const String& _fileName, const ShaderType& _shaderType)
{
	shader = std::make_unique<sf::Shader>(std::filesystem::path(_fileName.StdString()), _shaderType);
}

Bool Krampus::Shader::IsAvaliable()
{
	return sf::Shader::isAvailable();
}

Bool Krampus::Shader::IsGeometryAvaliable()
{
	return sf::Shader::isGeometryAvailable();
}

void Krampus::Shader::Bind(const Shader* _shader)
{
	sf::Shader::bind(!_shader ? nullptr : _shader->shader.get());
}

void Krampus::Shader::Bind()
{
	sf::Shader::bind(shader.get());
}

Bool Krampus::Shader::Load(const String& _vertexShaderFileName, const String& _fragmentShaderFileName)
{
	return shader->loadFromFile(std::filesystem::path(_vertexShaderFileName.StdString()),
		std::filesystem::path(_fragmentShaderFileName.StdString()));
}

Bool Krampus::Shader::Load(const String& _vertexShaderFileName, const String& _geometryShaderFileName, const String& _fragmentShaderFileName)
{
	return shader->loadFromFile(std::filesystem::path(_vertexShaderFileName.StdString()),
		std::filesystem::path(_geometryShaderFileName.StdString()),
		std::filesystem::path(_fragmentShaderFileName.StdString()));
}

Bool Krampus::Shader::Load(const String& _fileName, const ShaderType& _type)
{
	return shader->loadFromFile(std::filesystem::path(_fileName.StdString()), _type);
}

void Krampus::Shader::SetUniform(const String& name, float x)
{
	shader->setUniform(name, x);
}

void Krampus::Shader::SetUniform(const String& name, sf::Glsl::Vec2 vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const String& name, const sf::Glsl::Vec3& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const String& name, const sf::Glsl::Vec4& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const String& name, int x)
{
	shader->setUniform(name, x);
}

void Krampus::Shader::SetUniform(const String& name, sf::Glsl::Ivec2 vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const String& name, const sf::Glsl::Ivec3& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const String& name, const sf::Glsl::Ivec4& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const String& name, bool x)
{
	shader->setUniform(name, x);
}

void Krampus::Shader::SetUniform(const String& name, sf::Glsl::Bvec2 vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const String& name, const sf::Glsl::Bvec3& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const String& name, const sf::Glsl::Bvec4& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const String& name, const sf::Glsl::Mat3& matrix)
{
	shader->setUniform(name, matrix);
}

void Krampus::Shader::SetUniform(const String& name, const sf::Glsl::Mat4& matrix)
{
	shader->setUniform(name, matrix);
}

void Krampus::Shader::SetUniform(const String& name, const sf::Texture& texture)
{
	shader->setUniform(name, texture);
}

void Krampus::Shader::SetUniform(const String& name, ShaderTextureType)
{
	shader->setUniform(name, sf::Shader::CurrentTexture);
}

void Krampus::Shader::SetUniformArray(const String& name, const float* scalarArray, std::size_t length)
{
	shader->setUniformArray(name, scalarArray, length);
}

void Krampus::Shader::SetUniformArray(const String& name, const sf::Glsl::Vec2* vectorArray, std::size_t length)
{
	shader->setUniformArray(name, vectorArray, length);
}

void Krampus::Shader::SetUniformArray(const String& name, const sf::Glsl::Vec3* vectorArray, std::size_t length)
{
	shader->setUniformArray(name, vectorArray, length);
}

void Krampus::Shader::SetUniformArray(const String& name, const sf::Glsl::Vec4* vectorArray, std::size_t length)
{
	shader->setUniformArray(name, vectorArray, length);
}

void Krampus::Shader::SetUniformArray(const String& name, const sf::Glsl::Mat3* matrixArray, std::size_t length)
{
	shader->setUniformArray(name, matrixArray, length);
}

void Krampus::Shader::SetUniformArray(const String& name, const sf::Glsl::Mat4* matrixArray, std::size_t length)
{
	shader->setUniformArray(name, matrixArray, length);
}
