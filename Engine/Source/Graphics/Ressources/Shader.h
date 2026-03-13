#pragma once
#include "Utilities/Macro.h"

namespace Krampus
{
	using ShaderType = sf::Shader::Type;
	using ShaderTextureType = sf::Shader::CurrentTextureType;

	class Shader
	{
		std::unique_ptr<sf::Shader>     shader;

	public:
        sf::Shader* GetSFShader()
        {
            return shader.get();
        }

		Shader() = default;
        Shader(const String& _vertexShaderFileName, const String& _fragmentShaderFileName);
        Shader(const String& _vertexShaderFileName, const String& _geometryShaderFileName, const String& _fragmentShaderFileName);
        Shader(const String& _fileName, const ShaderType& _shaderType);

		static Bool IsAvaliable();
		static Bool IsGeometryAvaliable();
        static void Bind(const Shader* _shader);
        void Bind();
        Bool Load(const String& _vertexShaderFileName, const String& _fragmentShaderFileName);
        Bool Load(const String& _vertexShaderFileName, const String& _geometryShaderFileName, const String& _fragmentShaderFileName);
        Bool Load(const String& _fileName, const ShaderType& _type);
        
        void SetUniform(const String& name, float x);
        void SetUniform(const String& name, sf::Glsl::Vec2 vector);
        void SetUniform(const String& name, const sf::Glsl::Vec3& vector);
        void SetUniform(const String& name, const sf::Glsl::Vec4& vector);
        void SetUniform(const String& name, int x);
        void SetUniform(const String& name, sf::Glsl::Ivec2 vector);
        void SetUniform(const String& name, const sf::Glsl::Ivec3& vector);
        void SetUniform(const String& name, const sf::Glsl::Ivec4& vector);
        void SetUniform(const String& name, bool x);
        void SetUniform(const String& name, sf::Glsl::Bvec2 vector);
        void SetUniform(const String& name, const sf::Glsl::Bvec3& vector);
        void SetUniform(const String& name, const sf::Glsl::Bvec4& vector);
        void SetUniform(const String& name, const sf::Glsl::Mat3& matrix);
        void SetUniform(const String& name, const sf::Glsl::Mat4& matrix);
        void SetUniform(const String& name, const sf::Texture& texture);
        void SetUniform(const String& name, ShaderTextureType);

        void SetUniformArray(const String& name, const float* scalarArray, std::size_t length);
        void SetUniformArray(const String& name, const sf::Glsl::Vec2* vectorArray, std::size_t length);
        void SetUniformArray(const String& name, const sf::Glsl::Vec3* vectorArray, std::size_t length);
        void SetUniformArray(const String& name, const sf::Glsl::Vec4* vectorArray, std::size_t length);
        void SetUniformArray(const String& name, const sf::Glsl::Mat3* matrixArray, std::size_t length);
        void SetUniformArray(const String& name, const sf::Glsl::Mat4* matrixArray, std::size_t length);
    };

}