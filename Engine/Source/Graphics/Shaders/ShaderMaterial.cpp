#include "ShaderMaterial.h"
#include "Managers/TextureManager.h"
#include <SFML/Graphics/Glsl.hpp>
#include <sstream>

using namespace Krampus;

namespace
{
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

	float At(const std::vector<float>& _numbers, const size_t _index, const float _otherwise)
	{
		return _index < _numbers.size() ? _numbers[_index] : _otherwise;
	}
}

std::string ShaderMaterial::ValueOf(const ShaderParam& _param) const
{
	const auto _found = values.find(_param.name);
	return _found != values.end() ? _found->second : _param.value;
}

bool ShaderMaterial::Apply(sf::RenderStates& _states, const sf::Texture* _texture, const sf::Vector2f& _resolution) const
{
	if (shader.empty() || !ShaderLibrary::Available()) return false;
	const ShaderAsset* _asset = ShaderLibrary::Get(shader);
	if (!_asset || !_asset->IsReady()) return false;
	sf::Shader& _shader = *_asset->shader;

	// What the engine gives (only what the shader declares: SFML complains about the others).
	if (_asset->usesTexture) _shader.setUniform("texture", sf::Shader::CurrentTexture);
	if (_asset->usesTextureSize) _shader.setUniform("textureSize", _texture ? sf::Glsl::Vec2(sf::Vector2f(_texture->getSize())) : sf::Glsl::Vec2(1.f, 1.f));
	if (_asset->usesResolution) _shader.setUniform("resolution", sf::Glsl::Vec2(_resolution));
	if (_asset->usesTime) _shader.setUniform("time", ShaderLibrary::Time());

	for (const ShaderParam& _param : _asset->params)
	{
		const std::string _text = ValueOf(_param);
		const std::vector<float> _n = _param.type == ShaderParam::Type::Texture ? std::vector<float>() : Numbers(_text);
		switch (_param.type)
		{
		case ShaderParam::Type::Float:		_shader.setUniform(_param.name, At(_n, 0, 0.f)); break;
		case ShaderParam::Type::Int:		_shader.setUniform(_param.name, static_cast<int>(At(_n, 0, 0.f))); break;
		case ShaderParam::Type::Bool:		_shader.setUniform(_param.name, _text == "true" || _text == "1"); break;
		case ShaderParam::Type::Vector2:	_shader.setUniform(_param.name, sf::Glsl::Vec2(At(_n, 0, 0.f), At(_n, 1, 0.f))); break;
		case ShaderParam::Type::Color3:		_shader.setUniform(_param.name, sf::Glsl::Vec3(At(_n, 0, 1.f), At(_n, 1, 1.f), At(_n, 2, 1.f))); break;
		case ShaderParam::Type::Color4:		_shader.setUniform(_param.name, sf::Glsl::Vec4(At(_n, 0, 1.f), At(_n, 1, 1.f), At(_n, 2, 1.f), At(_n, 3, 1.f))); break;
		case ShaderParam::Type::Texture:
		{
			if (_text.empty()) break;
			auto& _picture = textures[_text];	// loaded once (a missing file is asked once too)
			if (!_picture && !textures.contains("?" + _text))
			{
				_picture = M_TEXTURE.GetSharedTexture(String(_text));
				if (!_picture) textures["?" + _text] = nullptr;
			}
			if (_picture) _shader.setUniform(_param.name, *_picture);
			break;
		}
		}
	}
	_states.shader = &_shader;
	return true;
}

std::string ShaderMaterial::EncodeValues() const
{
	std::string _text;
	for (const auto& [_name, _value] : values)
	{
		std::string _clean = _value;
		std::replace(_clean.begin(), _clean.end(), '|', '/');
		if (!_text.empty()) _text += '|';
		_text += _name + "=" + _clean;
	}
	return _text;
}

void ShaderMaterial::DecodeValues(const std::string& _text)
{
	values.clear();
	std::istringstream _in(_text);
	std::string _pair;
	while (std::getline(_in, _pair, '|'))
	{
		const size_t _equal = _pair.find('=');
		if (_equal != std::string::npos && _equal > 0) values[_pair.substr(0, _equal)] = _pair.substr(_equal + 1);
	}
}
