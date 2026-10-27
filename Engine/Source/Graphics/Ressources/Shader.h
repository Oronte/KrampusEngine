#pragma once
#include "Utilities/Macro.h"

namespace Krampus
{
	/// <summary>
	/// Type alias representing the stage/pipeline of a GLSL shader (Vertex, Geometry, Fragment).
	/// </summary>
	using ShaderType = sf::Shader::Type;

	/// <summary>
	/// Type alias representing the current texture uniform in a shader.
	/// </summary>
	using ShaderTextureType = sf::Shader::CurrentTextureType;

	/// <summary>
	/// Manages GLSL shader programs for custom GPU vertex, geometry, and fragment effects.
	/// </summary>
	class Shader
	{
		std::unique_ptr<sf::Shader>     shader;

	public:
		/// <summary>
		/// Gets a raw pointer to the underlying SFML shader.
		/// </summary>
		/// <returns>Pointer to sf::Shader.</returns>
        sf::Shader* GetSFShader()
        {
            return shader.get();
        }

		/// <summary>
		/// Default constructor for Shader.
		/// </summary>
		Shader() = default;

		/// <summary>
		/// Constructs and loads a shader program combining a vertex and fragment shader.
		/// </summary>
		/// <param name="_vertexShaderFileName">Vertex shader file path.</param>
		/// <param name="_fragmentShaderFileName">Fragment shader file path.</param>
        Shader(const String& _vertexShaderFileName, const String& _fragmentShaderFileName);

		/// <summary>
		/// Constructs and loads a shader program combining vertex, geometry, and fragment shaders.
		/// </summary>
		/// <param name="_vertexShaderFileName">Vertex shader file path.</param>
		/// <param name="_geometryShaderFileName">Geometry shader file path.</param>
		/// <param name="_fragmentShaderFileName">Fragment shader file path.</param>
        Shader(const String& _vertexShaderFileName, const String& _geometryShaderFileName, const String& _fragmentShaderFileName);

		/// <summary>
		/// Constructs and loads a single-stage shader from a file.
		/// </summary>
		/// <param name="_fileName">Shader source file path.</param>
		/// <param name="_shaderType">Type of shader stage.</param>
        Shader(const String& _fileName, const ShaderType& _shaderType);

		/// <summary>
		/// Checks whether programmable shaders are supported by the host GPU and drivers.
		/// </summary>
		/// <returns>True if shaders are supported.</returns>
		static Bool IsAvaliable();

		/// <summary>
		/// Checks whether geometry shaders are supported by the host GPU.
		/// </summary>
		/// <returns>True if geometry shaders are supported.</returns>
		static Bool IsGeometryAvaliable();

		/// <summary>
		/// Binds a shader program for subsequent draw calls.
		/// </summary>
		/// <param name="_shader">Pointer to Shader to bind, or nullptr to unbind.</param>
        static void Bind(const Shader* _shader);

		/// <summary>
		/// Binds this shader program for subsequent draw calls.
		/// </summary>
        void Bind();

		/// <summary>
		/// Loads vertex and fragment shaders from files.
		/// </summary>
		/// <param name="_vertexShaderFileName">Vertex shader file.</param>
		/// <param name="_fragmentShaderFileName">Fragment shader file.</param>
		/// <returns>True if loaded successfully.</returns>
        Bool Load(const String& _vertexShaderFileName, const String& _fragmentShaderFileName);

		/// <summary>
		/// Loads vertex, geometry, and fragment shaders from files.
		/// </summary>
		/// <param name="_vertexShaderFileName">Vertex shader file.</param>
		/// <param name="_geometryShaderFileName">Geometry shader file.</param>
		/// <param name="_fragmentShaderFileName">Fragment shader file.</param>
		/// <returns>True if loaded successfully.</returns>
        Bool Load(const String& _vertexShaderFileName, const String& _geometryShaderFileName, const String& _fragmentShaderFileName);

		/// <summary>
		/// Loads a single shader stage from a file.
		/// </summary>
		/// <param name="_fileName">Shader source file path.</param>
		/// <param name="_type">Stage type.</param>
		/// <returns>True if loaded successfully.</returns>
        Bool Load(const String& _fileName, const ShaderType& _type);
        
		/// <summary>Sets a float uniform variable in the shader.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="x">Float value.</param>
        void SetUniform(const String& name, float x);

		/// <summary>Sets a 2D float vector uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vector">Vec2 value.</param>
        void SetUniform(const String& name, sf::Glsl::Vec2 vector);

		/// <summary>Sets a 3D float vector uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vector">Vec3 value.</param>
        void SetUniform(const String& name, const sf::Glsl::Vec3& vector);

		/// <summary>Sets a 4D float vector uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vector">Vec4 value.</param>
        void SetUniform(const String& name, const sf::Glsl::Vec4& vector);

		/// <summary>Sets an integer uniform variable in the shader.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="x">Integer value.</param>
        void SetUniform(const String& name, int x);

		/// <summary>Sets a 2D integer vector uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vector">Ivec2 value.</param>
        void SetUniform(const String& name, sf::Glsl::Ivec2 vector);

		/// <summary>Sets a 3D integer vector uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vector">Ivec3 value.</param>
        void SetUniform(const String& name, const sf::Glsl::Ivec3& vector);

		/// <summary>Sets a 4D integer vector uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vector">Ivec4 value.</param>
        void SetUniform(const String& name, const sf::Glsl::Ivec4& vector);

		/// <summary>Sets a boolean uniform variable in the shader.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="x">Boolean value.</param>
        void SetUniform(const String& name, bool x);

		/// <summary>Sets a 2D boolean vector uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vector">Bvec2 value.</param>
        void SetUniform(const String& name, sf::Glsl::Bvec2 vector);

		/// <summary>Sets a 3D boolean vector uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vector">Bvec3 value.</param>
        void SetUniform(const String& name, const sf::Glsl::Bvec3& vector);

		/// <summary>Sets a 4D boolean vector uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vector">Bvec4 value.</param>
        void SetUniform(const String& name, const sf::Glsl::Bvec4& vector);

		/// <summary>Sets a 3x3 matrix uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="matrix">Mat3 matrix.</param>
        void SetUniform(const String& name, const sf::Glsl::Mat3& matrix);

		/// <summary>Sets a 4x4 matrix uniform variable.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="matrix">Mat4 matrix.</param>
        void SetUniform(const String& name, const sf::Glsl::Mat4& matrix);

		/// <summary>Sets a sampler2D texture uniform variable from a sf::Texture.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="texture">SFML texture reference.</param>
        void SetUniform(const String& name, const sf::Texture& texture);

		/// <summary>Sets a sampler2D texture uniform variable to use the currently bound texture.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="_type">CurrentTextureType indicator.</param>
        void SetUniform(const String& name, ShaderTextureType _type);

		/// <summary>Sets an array of scalar float uniforms.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="scalarArray">Pointer to float array.</param>
		/// <param name="length">Number of elements.</param>
        void SetUniformArray(const String& name, const float* scalarArray, std::size_t length);

		/// <summary>Sets an array of 2D float vector uniforms.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vectorArray">Pointer to Vec2 array.</param>
		/// <param name="length">Number of elements.</param>
        void SetUniformArray(const String& name, const sf::Glsl::Vec2* vectorArray, std::size_t length);

		/// <summary>Sets an array of 3D float vector uniforms.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vectorArray">Pointer to Vec3 array.</param>
		/// <param name="length">Number of elements.</param>
        void SetUniformArray(const String& name, const sf::Glsl::Vec3* vectorArray, std::size_t length);

		/// <summary>Sets an array of 4D float vector uniforms.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="vectorArray">Pointer to Vec4 array.</param>
		/// <param name="length">Number of elements.</param>
        void SetUniformArray(const String& name, const sf::Glsl::Vec4* vectorArray, std::size_t length);

		/// <summary>Sets an array of 3x3 matrix uniforms.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="matrixArray">Pointer to Mat3 array.</param>
		/// <param name="length">Number of elements.</param>
        void SetUniformArray(const String& name, const sf::Glsl::Mat3* matrixArray, std::size_t length);

		/// <summary>Sets an array of 4x4 matrix uniforms.</summary>
		/// <param name="name">Uniform name.</param>
		/// <param name="matrixArray">Pointer to Mat4 array.</param>
		/// <param name="length">Number of elements.</param>
        void SetUniformArray(const String& name, const sf::Glsl::Mat4* matrixArray, std::size_t length);
    };

}
