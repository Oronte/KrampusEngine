#include "ShaderLibrary.h"
#include "Core/AssetPack.h"
#include "Utilities/Debug/Logger.h"
#include <SFML/System/Err.hpp>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>

using namespace Krampus;

namespace
{
	struct Entry
	{
		std::unique_ptr<ShaderAsset>		asset;
		std::filesystem::file_time_type	fragmentTime;
		std::filesystem::file_time_type	vertexTime;
		bool								hadVertex	= false;
	};

	std::map<std::string, Entry>& Cache()
	{
		static std::map<std::string, Entry> _cache;
		return _cache;
	}

	const std::chrono::steady_clock::time_point& StartTime()
	{
		static const std::chrono::steady_clock::time_point _start = std::chrono::steady_clock::now();
		return _start;
	}

	std::string ReadText(const std::filesystem::path& _path)
	{
		std::unique_ptr<std::istream> _file = AssetPack::OpenStream(_path);	// a packed game has its files in its exe
		if (!_file || !*_file) return "";
		std::ostringstream _text;
		_text << _file->rdbuf();
		return _text.str();
	}

	std::filesystem::file_time_type TimeOf(const std::filesystem::path& _path)
	{
		std::error_code _error;
		const auto _time = std::filesystem::last_write_time(_path, _error);
		return _error ? std::filesystem::file_time_type() : _time;
	}

	std::string Trim(const std::string& _text)
	{
		const size_t _a = _text.find_first_not_of(" \t\r\n");
		if (_a == std::string::npos) return "";
		return _text.substr(_a, _text.find_last_not_of(" \t\r\n") - _a + 1);
	}

	bool IsBuiltin(const std::string& _name)
	{
		return _name == "texture" || _name == "textureSize" || _name == "resolution" || _name == "time";
	}

	// Compiles, with what SFML writes to sf::err() kept as the message.
	std::string CompileInto(sf::Shader& _shader, const std::string& _fragment, const std::string& _vertex)
	{
		std::ostringstream _messages;
		std::streambuf* const _previous = sf::err().rdbuf(_messages.rdbuf());
		const bool _ok = _vertex.empty() ? _shader.loadFromMemory(_fragment, sf::Shader::Type::Fragment) : _shader.loadFromMemory(_vertex, _fragment);
		sf::err().rdbuf(_previous);
		if (_ok) return "";
		const std::string _text = Trim(_messages.str());
		return _text.empty() ? std::string("the shader does not compile") : _text;
	}

	void CompileSources(const std::string& _name, ShaderAsset& _asset);

	void Compile(const std::string& _name, Entry& _entry)
	{
		if (!_entry.asset) _entry.asset = std::make_unique<ShaderAsset>();
		ShaderAsset& _asset = *_entry.asset;
		_asset.fragmentSource = ReadText(ShaderLibrary::FragmentPath(_name));
		_entry.hadVertex = AssetPack::Exists(ShaderLibrary::VertexPath(_name));
		_asset.vertexSource = _entry.hadVertex ? ReadText(ShaderLibrary::VertexPath(_name)) : std::string();
		_entry.fragmentTime = TimeOf(ShaderLibrary::FragmentPath(_name));
		_entry.vertexTime = TimeOf(ShaderLibrary::VertexPath(_name));
		CompileSources(_name, _asset);
	}

	void CompileSources(const std::string& _name, ShaderAsset& _asset)
	{
		_asset.name = _name;
		++_asset.version;

		if (_asset.fragmentSource.empty())
		{
			_asset.shader.reset();
			_asset.params.clear();
			_asset.error = "no file " + ShaderLibrary::FragmentPath(_name).generic_string();
			return;
		}

		const std::string _all = _asset.vertexSource + "\n" + _asset.fragmentSource;
		_asset.params = ShaderLibrary::ParseParams(_all);
		const auto _declares = [&](const char* _uniform) { return std::regex_search(_all, std::regex(std::string("uniform\\s+\\w+\\s+") + _uniform + "\\s*;")); };
		_asset.usesTexture = _declares("texture");
		_asset.usesTextureSize = _declares("textureSize");
		_asset.usesResolution = _declares("resolution");
		_asset.usesTime = _declares("time");

		if (!ShaderLibrary::Available())
		{
			_asset.shader.reset();
			_asset.error = "this graphics card has no shaders";
			return;
		}
		auto _shader = std::make_unique<sf::Shader>();
		_asset.error = CompileInto(*_shader, _asset.fragmentSource, _asset.vertexSource);
		_asset.shader = _asset.error.empty() ? std::move(_shader) : nullptr;
		if (_name.rfind("__", 0) == 0) return;	// a preview: its errors are shown where it is edited
		if (_asset.error.empty()) LOG(VerbosityType::Log, "Shader: " + _name + " compiled (" + std::to_string(_asset.params.size()) + " parameters)");
		else LOG_WARNING("Shader: " + _name + " does not compile:\n" + _asset.error);
	}
}

std::filesystem::path ShaderLibrary::Folder()							{ return std::filesystem::path("Content") / "Shaders"; }
std::filesystem::path ShaderLibrary::FragmentPath(const std::string& _name)	{ return Folder() / (_name + FRAGMENT); }
std::filesystem::path ShaderLibrary::VertexPath(const std::string& _name)		{ return Folder() / (_name + VERTEX); }

bool ShaderLibrary::Available()
{
	static const bool _available = sf::Shader::isAvailable();
	return _available;
}

float ShaderLibrary::Time()
{
	return std::chrono::duration<float>(std::chrono::steady_clock::now() - StartTime()).count();
}

Event<const std::string&>& ShaderLibrary::OnChanged()
{
	static Event<const std::string&> _event;
	return _event;
}

const ShaderAsset* ShaderLibrary::Get(const std::string& _name)
{
	if (_name.empty()) return nullptr;
	auto& _cache = Cache();
	auto _found = _cache.find(_name);
	if (_found == _cache.end())
	{
		StartTime();
		_found = _cache.emplace(_name, Entry()).first;
		Compile(_name, _found->second);
	}
	return _found->second.asset->fragmentSource.empty() ? nullptr : _found->second.asset.get();
}

std::vector<std::string> ShaderLibrary::List()
{
	std::vector<std::string> _names;
	std::error_code _error;
	if (!std::filesystem::is_directory(Folder(), _error)) return _names;
	for (const auto& _entry : std::filesystem::recursive_directory_iterator(Folder(), _error))
	{
		if (!_entry.is_regular_file() || _entry.path().extension() != FRAGMENT) continue;
		std::filesystem::path _relative = std::filesystem::relative(_entry.path(), Folder(), _error);
		_relative.replace_extension();
		_names.push_back(_relative.generic_string());
	}
	std::sort(_names.begin(), _names.end());
	return _names;
}

bool ShaderLibrary::Save(const std::string& _name, const std::string& _fragment, const std::string& _vertex)
{
	std::error_code _error;
	std::filesystem::create_directories(FragmentPath(_name).parent_path(), _error);
	{
		std::ofstream _file(FragmentPath(_name), std::ios::trunc | std::ios::binary);
		if (!_file.is_open()) return false;
		_file << _fragment;
	}
	if (Trim(_vertex).empty()) std::filesystem::remove(VertexPath(_name), _error);
	else
	{
		std::ofstream _file(VertexPath(_name), std::ios::trunc | std::ios::binary);
		_file << _vertex;
	}
	Entry& _entry = Cache()[_name];
	Compile(_name, _entry);
	const std::string _copy = _name;
	OnChanged().Broadcast(_copy);
	return _entry.asset->IsReady();
}

const ShaderAsset* ShaderLibrary::CompileFromSource(const std::string& _key, const std::string& _fragment, const std::string& _vertex)
{
	Entry& _entry = Cache()[_key];
	if (!_entry.asset) _entry.asset = std::make_unique<ShaderAsset>();
	_entry.asset->fragmentSource = _fragment;
	_entry.asset->vertexSource = _vertex;
	CompileSources(_key, *_entry.asset);
	return _entry.asset.get();
}

std::string ShaderLibrary::TryCompile(const std::string& _fragment, const std::string& _vertex, sf::Shader& _out)
{
	if (!Available()) return "this graphics card has no shaders";
	return CompileInto(_out, _fragment, _vertex);
}

void ShaderLibrary::CheckChanges()
{
	static auto _last = std::chrono::steady_clock::now();
	const auto _now = std::chrono::steady_clock::now();
	if (std::chrono::duration<float>(_now - _last).count() < 1.f) return;
	_last = _now;

	for (auto& [_name, _entry] : Cache())
	{
		if (_name.rfind("__", 0) == 0) continue;	// a preview has no file
		const bool _hasVertex = std::filesystem::exists(VertexPath(_name));
		if (TimeOf(FragmentPath(_name)) == _entry.fragmentTime && TimeOf(VertexPath(_name)) == _entry.vertexTime && _hasVertex == _entry.hadVertex) continue;
		Compile(_name, _entry);
		const std::string _copy = _name;
		OnChanged().Broadcast(_copy);
	}
}

std::vector<ShaderParam> ShaderLibrary::ParseParams(const std::string& _source)
{
	// "uniform <type> <name>;" and what the comment after it says: "default [min, max] help".
	static const std::regex UNIFORM(R"(^\s*uniform\s+(\w+)\s+(\w+)\s*;[ \t]*(?://(.*))?$)");
	static const std::regex RANGE(R"(\[\s*(-?[0-9.]+)\s*,\s*(-?[0-9.]+)\s*\])");
	static const std::regex NUMBERS(R"(^\s*(-?[0-9.]+(?:\s*,\s*-?[0-9.]+)*))");
	static const std::regex BOOLEAN(R"(^\s*(true|false))");
	static const std::regex PICTURE(R"(^\s*(\S+\.(?:png|jpg|jpeg|bmp|tga|PNG|JPG|JPEG|BMP|TGA)))");

	std::vector<ShaderParam> _params;
	std::istringstream _lines(_source);
	std::string _line;
	while (std::getline(_lines, _line))
	{
		if (!_line.empty() && _line.back() == '\r') _line.pop_back();
		std::smatch _match;
		if (!std::regex_match(_line, _match, UNIFORM)) continue;
		const std::string _type = _match[1], _name = _match[2];
		if (IsBuiltin(_name)) continue;
		if (std::any_of(_params.begin(), _params.end(), [&](const ShaderParam& _p) { return _p.name == _name; })) continue;	// in both stages

		ShaderParam _param;
		_param.name = _name;
		if (_type == "float")			{ _param.type = ShaderParam::Type::Float;	_param.value = "0"; }
		else if (_type == "int")		{ _param.type = ShaderParam::Type::Int;		_param.value = "0"; }
		else if (_type == "bool")		{ _param.type = ShaderParam::Type::Bool;	_param.value = "false"; }
		else if (_type == "vec2")		{ _param.type = ShaderParam::Type::Vector2;	_param.value = "0, 0"; }
		else if (_type == "vec3")		{ _param.type = ShaderParam::Type::Color3;	_param.value = "1, 1, 1"; }
		else if (_type == "vec4")		{ _param.type = ShaderParam::Type::Color4;	_param.value = "1, 1, 1, 1"; }
		else if (_type == "sampler2D")	{ _param.type = ShaderParam::Type::Texture;	_param.value = ""; }
		else continue;	// a type the Inspector cannot edit

		std::string _comment = _match[3].matched ? std::string(_match[3]) : std::string();
		std::smatch _found;
		if (std::regex_search(_comment, _found, RANGE))
		{
			_param.min = std::stof(_found[1]);
			_param.max = std::stof(_found[2]);
			_comment = _found.prefix().str() + " " + _found.suffix().str();
		}
		const std::regex& _valuePattern = _param.type == ShaderParam::Type::Bool ? BOOLEAN : _param.type == ShaderParam::Type::Texture ? PICTURE : NUMBERS;
		if (std::regex_search(_comment, _found, _valuePattern))
		{
			_param.value = Trim(_found[1]);
			_comment = _found.suffix().str();
		}
		_param.help = Trim(_comment);
		_params.push_back(std::move(_param));
	}
	return _params;
}
