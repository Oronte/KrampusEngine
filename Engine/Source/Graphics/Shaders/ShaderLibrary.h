#pragma once
#include "Utilities/System/Event.h"
#include <SFML/Graphics/Shader.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

// Shaders of the game: Content/Shaders/Name.frag (and Name.vert when it needs its own vertex stage), GLSL as SFML takes it.
//
//   Parameters are the uniforms of the file; a comment after one gives its default, range and help, shown in the Inspector:
//       uniform float strength;   // 0.5 [0, 2] how strong the effect is
//       uniform vec4 tint;        // 1, 0.2, 0.2, 1 the color it takes   (vec4: a color with alpha, vec3: a color)
//       uniform sampler2D noise;  // Builtin/Noise.png                   (a picture of Content/Textures)
//
//   Given by the engine to every shader that declares them (never shown as parameters):
//       texture       the picture being drawn (the screen for a post-process effect)
//       textureSize   its size in pixels
//       resolution    the size of the window
//       time          seconds since the program started
//
// Files are read once and kept; in the editor a changed file is compiled again by itself (CheckChanges).

namespace Krampus
{
	/// <summary>A parameter of a shader: a uniform it declares, with its default and range.</summary>
	struct ShaderParam
	{
		enum class Type { Float, Int, Bool, Vector2, Color3, Color4, Texture };

		std::string		name;
		Type			type		= Type::Float;
		std::string		value;					// default, as text ("0.5", "1, 0, 0, 1", "Folder/Name.png")
		float			min			= 0.f;		// min == max: no range
		float			max			= 0.f;
		std::string		help;
	};

	/// <summary>A shader of Content/Shaders, compiled.</summary>
	struct ShaderAsset
	{
		std::string					name;
		std::unique_ptr<sf::Shader>	shader;			// null when it did not compile
		std::vector<ShaderParam>	params;
		std::string					error;			// what the compiler said, empty when it compiled
		std::string					fragmentSource;
		std::string					vertexSource;	// empty: SFML's own vertex stage
		int							version		= 0;	// goes up at each compilation

		// The built-in uniforms it declares (only those are set).
		bool usesTexture = false, usesTextureSize = false, usesResolution = false, usesTime = false;

		bool IsReady() const { return shader != nullptr; }
	};

	class ShaderLibrary
	{
	public:
		static constexpr const char*	FRAGMENT	= ".frag";
		static constexpr const char*	VERTEX		= ".vert";

		/// <returns>Content/Shaders.</returns>
		static std::filesystem::path Folder();
		static std::filesystem::path FragmentPath(const std::string& _name);
		static std::filesystem::path VertexPath(const std::string& _name);

		/// <returns>The shader (compiled the first time), or null when there is no such file. Check IsReady: it may not compile.</returns>
		static const ShaderAsset* Get(const std::string& _name);

		/// <returns>Every shader of Content/Shaders (names with their folders), sorted.</returns>
		static std::vector<std::string> List();

		/// <summary>Writes the sources and compiles them again (the Shader Editor). Returns false when it does not compile (see Get()->error).</summary>
		static bool Save(const std::string& _name, const std::string& _fragment, const std::string& _vertex);

		/// <summary>
		/// Compiles sources that are not in a file, as a shader named _key (start it with "__": the Shader Editor's live preview).
		/// A ShaderMaterial can then use _key. Check IsReady and error.
		/// </summary>
		static const ShaderAsset* CompileFromSource(const std::string& _key, const std::string& _fragment, const std::string& _vertex);

		/// <summary>Compiles sources without saving them: returns the compiler's message, empty when it compiled.</summary>
		static std::string TryCompile(const std::string& _fragment, const std::string& _vertex, sf::Shader& _out);

		/// <summary>Compiles again the shaders whose files changed (the editor calls it; cheap: looks at the files every second).</summary>
		static void CheckChanges();

		/// <returns>The parameters a source declares (the uniforms that are not given by the engine).</returns>
		static std::vector<ShaderParam> ParseParams(const std::string& _source);

		/// <summary>Seconds since the program started: the "time" of every shader.</summary>
		static float Time();

		/// <summary>Said with the name of a shader compiled again.</summary>
		static Event<const std::string&>& OnChanged();

		/// <summary>Whether this computer can run shaders.</summary>
		static bool Available();
	};
}
