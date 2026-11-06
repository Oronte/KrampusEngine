#include "EditorContext.h"
#include "EditorWidgets.h"
#include "EditorTheme.h"
#include "Graphics/Shaders/ShaderMaterial.h"
#include "Managers/TextureManager.h"
#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace Krampus;

// The Shader Editor: the GLSL of a shader of Content/Shaders, compiled while it is typed, with a live preview and its parameters.
// Saving (Ctrl+S) writes the files: everything that uses the shader shows the change at once (and a file changed by another program
// is compiled again by itself).

namespace
{
	constexpr const char*	PREVIEW_KEY		= "__ShaderEditorPreview";
	constexpr unsigned		PREVIEW_SIZE	= 256;

	struct ShaderEditorState
	{
		bool					open			= false;
		std::string				name;					// the shader being edited
		std::string				fragment, vertex;	// its text, as typed
		std::string				savedFragment, savedVertex;
		std::string				compiledFragment, compiledVertex;	// what the preview was made from
		std::chrono::steady_clock::time_point	lastEdit;
		std::string				error;
		bool					compiled		= false;
		ShaderMaterial			preview;				// the preview's own parameter values
		char					picture[128]	= "_internal/KrampusEngineLogo.png";
		bool					fullPicture	= false;	// preview as a post-process effect: the shader covers the whole picture
		char					newName[64]	= "MyShader";
		int						tab			= 0;	// 0 fragment, 1 vertex
	} state;

	const char* SPRITE_TEMPLATE = R"(// A sprite shader. A parameter is a uniform with a comment: default [min, max] help.
uniform sampler2D texture;
uniform float time;
uniform float strength; // 1 [0, 2] how strong the effect is
uniform vec4 tint; // 1, 0.8, 0.6, 1 the color it pulses to

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec4 color = texture2D(texture, uv) * gl_Color;
	float pulse = 0.5 + 0.5 * sin(time * 3.0);
	gl_FragColor = vec4(mix(color.rgb, color.rgb * tint.rgb, pulse * strength * tint.a), color.a);
}
)";

	const char* POST_TEMPLATE = R"(// A post-process shader: texture is the picture of the world, resolution the size of the window.
uniform sampler2D texture;
uniform vec2 resolution;
uniform float strength; // 0.5 [0, 1] how much the colors are inverted

void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec3 color = texture2D(texture, uv).rgb;
	gl_FragColor = vec4(mix(color, 1.0 - color, strength), 1.0);
}
)";

	int ResizeString(ImGuiInputTextCallbackData* _data)
	{
		if (_data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			std::string* _text = static_cast<std::string*>(_data->UserData);
			_text->resize(static_cast<size_t>(_data->BufTextLen));
			_data->Buf = _text->data();
		}
		return 0;
	}

	bool CodeField(const char* _id, std::string& _text, const ImVec2& _size)
	{
		return ImGui::InputTextMultiline(_id, _text.data(), _text.capacity() + 1, _size,
			ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_AllowTabInput, ResizeString, &_text);
	}

	std::string Number(const float _value)
	{
		std::ostringstream _out;
		_out << std::setprecision(4) << _value;
		return _out.str();
	}

	std::vector<float> Numbers(const std::string& _text)
	{
		std::vector<float> _numbers;
		std::string _clean = _text;
		std::replace(_clean.begin(), _clean.end(), ',', ' ');
		std::istringstream _in(_clean);
		float _value;
		while (_in >> _value) _numbers.push_back(_value);
		return _numbers;
	}

	bool Dirty()
	{
		return state.fragment != state.savedFragment || state.vertex != state.savedVertex;
	}

	void Save()
	{
		if (state.name.empty()) return;
		const bool _ok = ShaderLibrary::Save(state.name, state.fragment, state.vertex);
		state.savedFragment = state.fragment;
		state.savedVertex = state.vertex;
		LOG_MSG("Shader " + state.name + " saved" + (_ok ? "" : " (it does not compile)"));
	}

	void Open(const std::string& _name)
	{
		if (Dirty()) Save();	// nothing typed is lost
		const ShaderAsset* _asset = ShaderLibrary::Get(_name);
		if (!_asset) return;
		state.name = _name;
		state.fragment = state.savedFragment = _asset->fragmentSource;
		state.vertex = state.savedVertex = _asset->vertexSource;
		state.compiledFragment.clear();
		state.compiledVertex = "\x01";	// compile the preview at once
		state.preview.values.clear();
		state.fullPicture = _name.find("Post/") != std::string::npos;
		state.tab = 0;
	}

	void Create(const std::string& _wanted, const bool _post)
	{
		std::string _name = (_post ? "Post/" : "") + (_wanted.empty() ? std::string("MyShader") : _wanted);
		for (int _n = 1; std::filesystem::exists(ShaderLibrary::FragmentPath(_name)); ++_n)
			_name = (_post ? "Post/" : "") + (_wanted.empty() ? std::string("MyShader") : _wanted) + std::to_string(_n);
		ShaderLibrary::Save(_name, _post ? POST_TEMPLATE : SPRITE_TEMPLATE, "");
		LOG_MSG("Shader Editor: created " + ShaderLibrary::FragmentPath(_name).generic_string());
		Open(_name);
	}

	// The preview: a checkerboard and the picture, drawn through the shader as it is typed.
	void DrawPreview()
	{
		static std::unique_ptr<sf::RenderTexture> _target, _scene;
		static std::unique_ptr<sf::Texture> _checker;
		if (!_target)
		{
			_target = std::make_unique<sf::RenderTexture>();
			_scene = std::make_unique<sf::RenderTexture>();
			if (!_target->resize({ PREVIEW_SIZE, PREVIEW_SIZE }) || !_scene->resize({ PREVIEW_SIZE, PREVIEW_SIZE })) return;
			sf::Image _image(sf::Vector2u(16, 16), sf::Color::Black);
			for (unsigned _y = 0; _y < 16; ++_y)
				for (unsigned _x = 0; _x < 16; ++_x)
					_image.setPixel({ _x, _y }, ((_x / 8 + _y / 8) % 2) ? sf::Color(70, 70, 78) : sf::Color(50, 50, 56));
			_checker = std::make_unique<sf::Texture>();
			if (!_checker->loadFromImage(_image)) return;
			_checker->setRepeated(true);
		}

		const std::shared_ptr<sf::Texture> _picture = M_TEXTURE.GetSharedTexture(String(state.picture));
		sf::Sprite _background(*_checker, sf::IntRect({ 0, 0 }, { static_cast<int>(PREVIEW_SIZE), static_cast<int>(PREVIEW_SIZE) }));
		const auto _fitted = [&]() -> std::optional<sf::Sprite>
		{
			if (!_picture) return std::nullopt;
			sf::Sprite _sprite(*_picture);
			const sf::Vector2f _size(_picture->getSize());
			const float _scale = std::min(220.f / _size.x, 220.f / _size.y);
			_sprite.setScale({ _scale, _scale });
			_sprite.setPosition({ (PREVIEW_SIZE - _size.x * _scale) * 0.5f, (PREVIEW_SIZE - _size.y * _scale) * 0.5f });
			return _sprite;
		};

		const sf::Vector2f _resolution(static_cast<float>(PREVIEW_SIZE), static_cast<float>(PREVIEW_SIZE));
		_target->clear();
		if (state.fullPicture)
		{
			// As a post-process effect: the whole picture goes through the shader.
			_scene->clear();
			_scene->draw(_background);
			if (auto _sprite = _fitted()) _scene->draw(*_sprite);
			_scene->display();
			sf::RenderStates _states;
			_states.blendMode = sf::BlendNone;
			state.preview.Apply(_states, &_scene->getTexture(), _resolution);
			_target->draw(sf::Sprite(_scene->getTexture()), _states);
		}
		else
		{
			_target->draw(_background);
			if (auto _sprite = _fitted())
			{
				sf::RenderStates _states;
				state.preview.Apply(_states, _picture.get(), _resolution);
				_target->draw(*_sprite, _states);
			}
		}
		_target->display();
		ImGui::Image(*_target, sf::Vector2f(static_cast<float>(PREVIEW_SIZE), static_cast<float>(PREVIEW_SIZE)));
		if (!_picture) ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::ConsoleWarning), "No picture Content/Textures/%s", state.picture);
	}
}

namespace Krampus
{
	bool DrawShaderParam(const ShaderParam& _param, std::string& _value)
	{
		ImGui::PushID(_param.name.c_str());
		bool _changed = false;
		const std::vector<float> _n = Numbers(_value);
		const auto _at = [&](const size_t _i, const float _otherwise) { return _i < _n.size() ? _n[_i] : _otherwise; };
		const char* _label = _param.name.c_str();
		switch (_param.type)
		{
		case ShaderParam::Type::Float:
		{
			float _v = _at(0, 0.f);
			_changed = _param.min < _param.max ? EditorUI::SliderFloat(_label, &_v, _param.min, _param.max) : ImGui::DragFloat(_label, &_v, 0.01f);
			if (_changed) _value = Number(_v);
			break;
		}
		case ShaderParam::Type::Int:
		{
			int _v = static_cast<int>(_at(0, 0.f));
			_changed = _param.min < _param.max ? EditorUI::SliderInt(_label, &_v, static_cast<int>(_param.min), static_cast<int>(_param.max)) : ImGui::DragInt(_label, &_v);
			if (_changed) _value = std::to_string(_v);
			break;
		}
		case ShaderParam::Type::Bool:
		{
			bool _v = _value == "true" || _value == "1";
			_changed = ImGui::Checkbox(_label, &_v);
			if (_changed) _value = _v ? "true" : "false";
			break;
		}
		case ShaderParam::Type::Vector2:
		{
			float _v[2] = { _at(0, 0.f), _at(1, 0.f) };
			_changed = ImGui::DragFloat2(_label, _v, 0.01f);
			if (_changed) _value = Number(_v[0]) + ", " + Number(_v[1]);
			break;
		}
		case ShaderParam::Type::Color3:
		{
			float _v[3] = { _at(0, 1.f), _at(1, 1.f), _at(2, 1.f) };
			_changed = ImGui::ColorEdit3(_label, _v, ImGuiColorEditFlags_Float);
			if (_changed) _value = Number(_v[0]) + ", " + Number(_v[1]) + ", " + Number(_v[2]);
			break;
		}
		case ShaderParam::Type::Color4:
		{
			float _v[4] = { _at(0, 1.f), _at(1, 1.f), _at(2, 1.f), _at(3, 1.f) };
			_changed = ImGui::ColorEdit4(_label, _v, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaBar);
			if (_changed) _value = Number(_v[0]) + ", " + Number(_v[1]) + ", " + Number(_v[2]) + ", " + Number(_v[3]);
			break;
		}
		case ShaderParam::Type::Texture:
		{
			char _buffer[256];
			std::snprintf(_buffer, sizeof(_buffer), "%s", _value.c_str());
			_changed = ImGui::InputTextWithHint(_label, "Folder/Picture.png (Content/Textures)", _buffer, sizeof(_buffer));
			if (_changed) _value = _buffer;
			break;
		}
		}
		if (!_param.help.empty() && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", _param.help.c_str());
		ImGui::PopID();
		return _changed;
	}

	void OpenShaderEditor(const std::string& _name)
	{
		state.open = true;
		if (!_name.empty() && _name != state.name) Open(_name);
		ImGui::SetWindowFocus("Shader Editor");
	}

	void SaveShaderEditorChanges()
	{
		if (state.open && Dirty()) Save();
	}

	void DrawShaderEditor(EditorContext&)
	{
		if (!state.open) return;
		ImGui::SetNextWindowSize(ImVec2(1180.f, 640.f), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Shader Editor", &state.open, ImGuiWindowFlags_MenuBar))
		{
			ImGui::End();
			return;
		}
		if (!state.open && Dirty()) Save();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Shader"))
			{
				if (ImGui::MenuItem("Save", "Ctrl+S", false, !state.name.empty())) Save();
				if (ImGui::MenuItem("Revert to the saved file", nullptr, false, Dirty())) { state.fragment = state.savedFragment; state.vertex = state.savedVertex; }
				ImGui::EndMenu();
			}
			if (!state.name.empty()) ImGui::TextDisabled("|  %s%s", state.name.c_str(), Dirty() ? " *" : "");
			if (!ShaderLibrary::Available()) ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::ConsoleError), "  this graphics card has no shaders");
			ImGui::EndMenuBar();
		}
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) Save();

		// ---- Left: the shaders, and new ones
		const float _scale = ImGui::GetFontSize() / 16.f;
		if (ImGui::BeginChild("##shaderList", ImVec2(230.f * _scale, 0.f), ImGuiChildFlags_Borders))
		{
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputText("##newName", state.newName, sizeof(state.newName));
			if (ImGui::Button("+ Sprite shader", ImVec2(-FLT_MIN, 0.f))) Create(state.newName, false);
			if (ImGui::Button("+ Post-process shader", ImVec2(-FLT_MIN, 0.f))) Create(state.newName, true);
			ImGui::SeparatorText("Content/Shaders");
			for (const std::string& _name : ShaderLibrary::List())
			{
				const ShaderAsset* _asset = ShaderLibrary::Get(_name);
				const bool _broken = _asset && !_asset->IsReady();
				if (_broken) ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Get(EditorTheme::Custom::ConsoleError));
				if (ImGui::Selectable(_name.c_str(), _name == state.name)) Open(_name);
				if (_broken) ImGui::PopStyleColor();
				if (_broken && ImGui::IsItemHovered()) ImGui::SetTooltip("Does not compile");
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();

		if (state.name.empty())
		{
			ImGui::TextWrapped("Pick a shader on the left, or make a new one. A shader is put on a sprite, a HUD image or particles in the "
				"Inspector (Material), or on the whole screen with a PostProcessComponent (Add Actor > PostProcessVolume).");
			ImGui::End();
			return;
		}

		// ---- Middle: the code, compiled a moment after the last key (the preview follows)
		const float _right = 300.f * _scale;
		if (ImGui::BeginChild("##shaderCode", ImVec2(ImGui::GetContentRegionAvail().x - _right - ImGui::GetStyle().ItemSpacing.x, 0.f)))
		{
			if (ImGui::BeginTabBar("##stages"))
			{
				if (ImGui::BeginTabItem("Fragment")) { state.tab = 0; ImGui::EndTabItem(); }
				if (ImGui::BeginTabItem(state.vertex.empty() ? "Vertex (none)" : "Vertex")) { state.tab = 1; ImGui::EndTabItem(); }
				ImGui::EndTabBar();
			}
			const float _errorHeight = ImGui::GetTextLineHeightWithSpacing() * (state.error.empty() ? 1.6f : 6.f);
			const ImVec2 _codeSize(-FLT_MIN, std::max(100.f, ImGui::GetContentRegionAvail().y - _errorHeight));
			const bool _edited = state.tab == 0 ? CodeField("##fragment", state.fragment, _codeSize) : CodeField("##vertex", state.vertex, _codeSize);
			if (_edited) state.lastEdit = std::chrono::steady_clock::now();
			if (state.tab == 1 && state.vertex.empty()) ImGui::TextDisabled("Empty: SFML's own vertex stage is used (the usual case).");

			const bool _changed = state.fragment != state.compiledFragment || state.vertex != state.compiledVertex;
			if (_changed && std::chrono::duration<float>(std::chrono::steady_clock::now() - state.lastEdit).count() > 0.35f)
			{
				const ShaderAsset* _asset = ShaderLibrary::CompileFromSource(PREVIEW_KEY, state.fragment, state.vertex);
				state.error = _asset ? _asset->error : std::string("no source");
				state.compiled = _asset && _asset->IsReady();
				state.compiledFragment = state.fragment;
				state.compiledVertex = state.vertex;
				state.preview.shader = PREVIEW_KEY;
			}
			if (state.error.empty()) ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::PlayButton), "Compiled%s", Dirty() ? "  (Ctrl+S to save it)" : "");
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Get(EditorTheme::Custom::ConsoleError));
				ImGui::BeginChild("##errors", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);
				ImGui::TextWrapped("%s", state.error.c_str());
				ImGui::EndChild();
				ImGui::PopStyleColor();
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();

		// ---- Right: the preview and its parameters
		if (ImGui::BeginChild("##shaderPreview", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders))
		{
			DrawPreview();
			ImGui::Checkbox("As a post-process effect", &state.fullPicture);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The whole picture goes through the shader, as the screen does with a PostProcessComponent");
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputTextWithHint("##picture", "Picture of Content/Textures", state.picture, sizeof(state.picture));

			ImGui::SeparatorText("Parameters (preview)");
			const ShaderAsset* _asset = ShaderLibrary::Get(PREVIEW_KEY);
			if (_asset && !_asset->params.empty())
			{
				for (const ShaderParam& _param : _asset->params)
				{
					std::string _value = state.preview.ValueOf(_param);
					if (DrawShaderParam(_param, _value)) state.preview.values[_param.name] = _value;
				}
				if (ImGui::Button("Defaults", ImVec2(-FLT_MIN, 0.f))) state.preview.values.clear();
			}
			else ImGui::TextDisabled("No parameter. Declare a uniform with a comment:\nuniform float strength; // 0.5 [0, 1]");
			ImGui::TextDisabled("Given by the engine: texture, textureSize,\nresolution, time.");
		}
		ImGui::EndChild();
		ImGui::End();
	}
}
