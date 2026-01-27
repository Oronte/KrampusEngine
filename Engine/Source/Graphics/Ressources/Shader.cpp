#include "Shader.h"

Krampus::Shader::Shader(const std::string& _vertexShaderFileName, const std::string& _fragmentShaderFileName)
{
	shader = std::make_unique<sf::Shader>(std::filesystem::path(_vertexShaderFileName),
		std::filesystem::path(_fragmentShaderFileName));
}

Krampus::Shader::Shader(const std::string& _vertexShaderFileName, const std::string& _geometryShaderFileName, const std::string& _fragmentShaderFileName)
{
	shader = std::make_unique<sf::Shader>(std::filesystem::path(_vertexShaderFileName),
		std::filesystem::path(_geometryShaderFileName),
		std::filesystem::path(_fragmentShaderFileName));
}

Krampus::Shader::Shader(const std::string& _fileName, const ShaderType& _shaderType)
{
	shader = std::make_unique<sf::Shader>(std::filesystem::path(_fileName), _shaderType);
}

bool Krampus::Shader::IsAvaliable()
{
	return sf::Shader::isAvailable();
}

bool Krampus::Shader::IsGeometryAvaliable()
{
	return sf::Shader::isGeometryAvailable();
}

bool Krampus::Shader::Load(const std::string& _vertexShaderFileName, const std::string& _fragmentShaderFileName)
{
	return shader->loadFromFile(std::filesystem::path(_vertexShaderFileName),
		std::filesystem::path(_fragmentShaderFileName));
}

bool Krampus::Shader::Load(const std::string& _vertexShaderFileName, const std::string& _geometryShaderFileName, const std::string& _fragmentShaderFileName)
{
	return shader->loadFromFile(std::filesystem::path(_vertexShaderFileName),
		std::filesystem::path(_geometryShaderFileName),
		std::filesystem::path(_fragmentShaderFileName));
}

bool Krampus::Shader::Load(const std::string& _fileName, const ShaderType& _type)
{
	return shader->loadFromFile(std::filesystem::path(_fileName), _type);
}

void Krampus::Shader::SetUniform(const std::string& name, float x)
{
	shader->setUniform(name, x);
}

void Krampus::Shader::SetUniform(const std::string& name, sf::Glsl::Vec2 vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const std::string& name, const sf::Glsl::Vec3& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const std::string& name, const sf::Glsl::Vec4& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const std::string& name, int x)
{
	shader->setUniform(name, x);
}

void Krampus::Shader::SetUniform(const std::string& name, sf::Glsl::Ivec2 vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const std::string& name, const sf::Glsl::Ivec3& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const std::string& name, const sf::Glsl::Ivec4& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const std::string& name, bool x)
{
	shader->setUniform(name, x);
}

void Krampus::Shader::SetUniform(const std::string& name, sf::Glsl::Bvec2 vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const std::string& name, const sf::Glsl::Bvec3& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const std::string& name, const sf::Glsl::Bvec4& vector)
{
	shader->setUniform(name, vector);
}

void Krampus::Shader::SetUniform(const std::string& name, const sf::Glsl::Mat3& matrix)
{
	shader->setUniform(name, matrix);
}

void Krampus::Shader::SetUniform(const std::string& name, const sf::Glsl::Mat4& matrix)
{
	shader->setUniform(name, matrix);
}

void Krampus::Shader::SetUniform(const std::string& name, const sf::Texture& texture)
{
	shader->setUniform(name, texture);
}

void Krampus::Shader::SetUniform(const std::string& name, ShaderTextureType)
{
	shader->setUniform(name, sf::Shader::CurrentTexture);
}

void Krampus::Shader::SetUniformArray(const std::string& name, const float* scalarArray, std::size_t length)
{
	shader->setUniformArray(name, scalarArray, length);
}

void Krampus::Shader::SetUniformArray(const std::string& name, const sf::Glsl::Vec2* vectorArray, std::size_t length)
{
	shader->setUniformArray(name, vectorArray, length);
}

void Krampus::Shader::SetUniformArray(const std::string& name, const sf::Glsl::Vec3* vectorArray, std::size_t length)
{
	shader->setUniformArray(name, vectorArray, length);
}

void Krampus::Shader::SetUniformArray(const std::string& name, const sf::Glsl::Vec4* vectorArray, std::size_t length)
{
	shader->setUniformArray(name, vectorArray, length);
}

void Krampus::Shader::SetUniformArray(const std::string& name, const sf::Glsl::Mat3* matrixArray, std::size_t length)
{
	shader->setUniformArray(name, matrixArray, length);
}

void Krampus::Shader::SetUniformArray(const std::string& name, const sf::Glsl::Mat4* matrixArray, std::size_t length)
{
	shader->setUniformArray(name, matrixArray, length);
}
