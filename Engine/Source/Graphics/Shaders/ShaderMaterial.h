#pragma once
#include "ShaderLibrary.h"
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <map>

namespace Krampus
{
	/// <summary>
	/// A shader and the values of its parameters, for one thing that is drawn (a sprite, an image, particles, a post-process effect).
	/// Values not set keep the defaults written in the shader file.
	/// </summary>
	class ShaderMaterial
	{
	public:
		std::string							shader;		// name in Content/Shaders, empty: none
		std::map<std::string, std::string>	values;		// parameter -> value as text

		bool HasShader() const { return !shader.empty(); }

		/// <summary>
		/// Gets the shader ready for a draw: its parameters, the built-in uniforms, and states.shader.
		/// _texture is what is drawn (its size is textureSize). False when there is no shader or it does not compile: draw normally.
		/// </summary>
		bool Apply(sf::RenderStates& _states, const sf::Texture* _texture, const sf::Vector2f& _resolution) const;

		/// <returns>The value of a parameter: the one set, else the shader's default.</returns>
		std::string ValueOf(const ShaderParam& _param) const;

		/// <summary>"name=value|name=value" (saved in scenes) and back.</summary>
		std::string EncodeValues() const;
		void DecodeValues(const std::string& _text);

		/// <summary>Sets one parameter (from code): SetParam("strength", "0.8").</summary>
		void SetParam(const std::string& _name, const std::string& _value) { values[_name] = _value; }

	private:
		mutable std::map<std::string, std::shared_ptr<sf::Texture>>	textures;	// pictures of the sampler2D parameters
	};

	/// <summary>Something drawn that can have a shader (the Inspector shows its Material section).</summary>
	class IShaded
	{
	public:
		ShaderMaterial	material;
		virtual ~IShaded() = default;
	};
}
