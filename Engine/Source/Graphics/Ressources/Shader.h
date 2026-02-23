#pragma once
#include "Utilities/Macro.h"

namespace Krampus
{
	using ShaderType = sf::Shader::Type;
	using ShaderTextureType = sf::Shader::CurrentTextureType;

	class Shader
	{
		std::unique_ptr<sf::Shader> shader;

	public:
        sf::Shader* GetSFShader()
        {
            return shader.get();
        }

		Shader() = default;
        Shader(const std::string& _vertexShaderFileName, const std::string& _fragmentShaderFileName);
        Shader(const std::string& _vertexShaderFileName, const std::string& _geometryShaderFileName, const std::string& _fragmentShaderFileName);
        Shader(const std::string& _fileName, const ShaderType& _shaderType);

		static bool IsAvaliable();
		static bool IsGeometryAvaliable();
        static void Bind(const Shader* _shader);
        void Bind();
		bool Load(const std::string& _vertexShaderFileName, const std::string& _fragmentShaderFileName);
		bool Load(const std::string& _vertexShaderFileName, const std::string& _geometryShaderFileName, const std::string& _fragmentShaderFileName);
		bool Load(const std::string& _fileName, const ShaderType& _type);
        
        void SetUniform(const std::string& name, float x);
        void SetUniform(const std::string& name, sf::Glsl::Vec2 vector);
        void SetUniform(const std::string& name, const sf::Glsl::Vec3& vector);
        void SetUniform(const std::string& name, const sf::Glsl::Vec4& vector);
        void SetUniform(const std::string& name, int x);
        void SetUniform(const std::string& name, sf::Glsl::Ivec2 vector);
        void SetUniform(const std::string& name, const sf::Glsl::Ivec3& vector);
        void SetUniform(const std::string& name, const sf::Glsl::Ivec4& vector);
        void SetUniform(const std::string& name, bool x);
        void SetUniform(const std::string& name, sf::Glsl::Bvec2 vector);
        void SetUniform(const std::string& name, const sf::Glsl::Bvec3& vector);
        void SetUniform(const std::string& name, const sf::Glsl::Bvec4& vector);
        void SetUniform(const std::string& name, const sf::Glsl::Mat3& matrix);
        void SetUniform(const std::string& name, const sf::Glsl::Mat4& matrix);
        void SetUniform(const std::string& name, const sf::Texture& texture);
        void SetUniform(const std::string& name, ShaderTextureType);
        
        void SetUniformArray(const std::string& name, const float* scalarArray, std::size_t length);
        void SetUniformArray(const std::string& name, const sf::Glsl::Vec2* vectorArray, std::size_t length);
        void SetUniformArray(const std::string& name, const sf::Glsl::Vec3* vectorArray, std::size_t length);
        void SetUniformArray(const std::string& name, const sf::Glsl::Vec4* vectorArray, std::size_t length);
        void SetUniformArray(const std::string& name, const sf::Glsl::Mat3* matrixArray, std::size_t length);
        void SetUniformArray(const std::string& name, const sf::Glsl::Mat4* matrixArray, std::size_t length);
    };

}