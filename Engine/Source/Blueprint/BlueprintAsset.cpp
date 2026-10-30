#include "Blueprint.h"
#include "Actors/Actor.h"
#include "Core/AssetPack.h"
#include "Utilities/Debug/Logger.h"
#include <algorithm>
#include <charconv>
#include <fstream>
#include <sstream>

using namespace Krampus;

// ================= Values =================

namespace
{
	float ParseFloat(const std::string& _text)
	{
		const size_t _start = _text.find_first_not_of(" \t");
		if (_start == std::string::npos) return 0.f;
		float _value = 0.f;
		std::from_chars(_text.data() + _start, _text.data() + _text.size(), _value);
		return _value;
	}

	std::vector<float> ParseNumbers(const std::string& _text)
	{
		std::vector<float> _numbers;
		std::string _clean = _text;
		std::replace(_clean.begin(), _clean.end(), ',', ' ');
		std::istringstream _in(_clean);
		float _value;
		while (_in >> _value) _numbers.push_back(_value);
		return _numbers;
	}

	std::string Number(const float& _value)
	{
		std::ostringstream _out;
		_out << _value;
		return _out.str();
	}
}

const char* BlueprintValues::TypeName(const PinType& _type)
{
	static const char* NAMES[] = { "Exec", "Bool", "Int", "Float", "String", "Vector2", "Color", "Actor" };
	return NAMES[std::clamp(static_cast<int>(_type), 0, 7)];
}

BpValue BlueprintValues::Default(const PinType& _type)
{
	switch (_type)
	{
	case PinType::Bool:		return false;
	case PinType::Int:		return 0;
	case PinType::Float:	return 0.f;
	case PinType::String:	return std::string();
	case PinType::Vector2:	return FVector2(0.f, 0.f);
	case PinType::Color:	return Color(255, 255, 255, 255);
	case PinType::Actor:	return static_cast<Actor*>(nullptr);
	default:				return false;
	}
}

std::string BlueprintValues::ToText(const BpValue& _value)
{
	switch (_value.index())
	{
	case 0: return std::get<bool>(_value) ? "true" : "false";
	case 1: return std::to_string(std::get<int>(_value));
	case 2: return Number(std::get<float>(_value));
	case 3: return std::get<std::string>(_value);
	case 4: { const FVector2& _v = std::get<FVector2>(_value); return "(" + Number(_v.x) + ", " + Number(_v.y) + ")"; }
	case 5: { const Color& _c = std::get<Color>(_value); return "(" + std::to_string(_c.r) + ", " + std::to_string(_c.g) + ", " + std::to_string(_c.b) + ", " + std::to_string(_c.a) + ")"; }
	default:
	{
		Actor* _actor = std::get<Actor*>(_value);
		return _actor ? _actor->GetName().StdString() : std::string("None");
	}
	}
}

BpValue BlueprintValues::Convert(const BpValue& _value, const PinType& _type)
{
	// Number of anything that has one.
	const auto _number = [&]() -> float
	{
		switch (_value.index())
		{
		case 0: return std::get<bool>(_value) ? 1.f : 0.f;
		case 1: return static_cast<float>(std::get<int>(_value));
		case 2: return std::get<float>(_value);
		case 3: return ParseFloat(std::get<std::string>(_value));
		case 4: return std::get<FVector2>(_value).Length();
		default: return 0.f;
		}
	};

	switch (_type)
	{
	case PinType::Bool:
		if (_value.index() == 0) return _value;
		if (_value.index() == 6) return std::get<Actor*>(_value) != nullptr;
		if (_value.index() == 3) { const std::string& _s = std::get<std::string>(_value); return _s == "true" || _s == "1" || _s == "True"; }
		return _number() != 0.f;
	case PinType::Int:
		if (_value.index() == 1) return _value;
		return static_cast<int>(std::lround(_number()));
	case PinType::Float:
		if (_value.index() == 2) return _value;
		return _number();
	case PinType::String:
		return ToText(_value);
	case PinType::Vector2:
		if (_value.index() == 4) return _value;
		if (_value.index() == 3) return Parse(std::get<std::string>(_value), PinType::Vector2);
		return FVector2(_number(), _number());
	case PinType::Color:
		if (_value.index() == 5) return _value;
		if (_value.index() == 3) return Parse(std::get<std::string>(_value), PinType::Color);
		return Color(255, 255, 255, 255);
	case PinType::Actor:
		if (_value.index() == 6) return _value;
		return static_cast<Actor*>(nullptr);
	default:
		return _value;
	}
}

bool BlueprintValues::CanConnect(const PinType& _from, const PinType& _to)
{
	if (_from == PinType::Exec || _to == PinType::Exec) return _from == _to;
	if (_from == _to || _to == PinType::String) return true;	// anything can be shown as text
	const auto _numeric = [](const PinType& _t) { return _t == PinType::Bool || _t == PinType::Int || _t == PinType::Float; };
	if (_numeric(_from) && _numeric(_to)) return true;
	if (_from == PinType::Actor && _to == PinType::Bool) return true;		// "is there one"
	if (_from == PinType::Vector2 && _to == PinType::Float) return true;	// its length
	return false;
}

BpValue BlueprintValues::Parse(const std::string& _text, const PinType& _type)
{
	switch (_type)
	{
	case PinType::Bool:		return _text == "true" || _text == "1" || _text == "True";
	case PinType::Int:		return static_cast<int>(std::lround(ParseFloat(_text)));
	case PinType::Float:	return ParseFloat(_text);
	case PinType::String:	return _text;
	case PinType::Vector2:
	{
		const std::vector<float> _n = ParseNumbers(_text);
		return FVector2(_n.size() > 0 ? _n[0] : 0.f, _n.size() > 1 ? _n[1] : 0.f);
	}
	case PinType::Color:
	{
		const std::vector<float> _n = ParseNumbers(_text);
		if (_n.size() < 3) return Color(255, 255, 255, 255);
		const auto _byte = [](const float _f) { return static_cast<std::uint8_t>(std::clamp(_f, 0.f, 255.f)); };
		return Color(_byte(_n[0]), _byte(_n[1]), _byte(_n[2]), _n.size() > 3 ? _byte(_n[3]) : 255);
	}
	default:				return Default(_type);
	}
}

std::string BlueprintValues::Format(const BpValue& _value)
{
	switch (_value.index())
	{
	case 0: return std::get<bool>(_value) ? "true" : "false";
	case 4: { const FVector2& _v = std::get<FVector2>(_value); return Number(_v.x) + ", " + Number(_v.y); }
	case 5: { const Color& _c = std::get<Color>(_value); return std::to_string(_c.r) + ", " + std::to_string(_c.g) + ", " + std::to_string(_c.b) + ", " + std::to_string(_c.a); }
	case 6: return "";
	default: return ToText(_value);
	}
}

// ================= Asset file =================
// "KBP 1", then one record per line, fields split by '|' ('\' escapes '|', '\', tabs and new lines):
//   V|name|type|value                               a variable
//   N|id|type|x|y|pin<TAB>value<TAB>pin<TAB>value     a node and what is typed in its inputs
//   L|fromNode|fromPin|toNode|toPin                  a wire

namespace
{
	std::string Escape(const std::string& _text)
	{
		std::string _out;
		for (const char _c : _text)
		{
			if (_c == '\\') _out += "\\\\";
			else if (_c == '|') _out += "\\p";
			else if (_c == '\t') _out += "\\t";
			else if (_c == '\n') _out += "\\n";
			else if (_c != '\r') _out += _c;
		}
		return _out;
	}

	std::string Unescape(const std::string& _text)
	{
		std::string _out;
		for (size_t _i = 0; _i < _text.size(); ++_i)
		{
			if (_text[_i] != '\\' || _i + 1 >= _text.size()) { _out += _text[_i]; continue; }
			const char _next = _text[++_i];
			_out += _next == 'p' ? '|' : _next == 't' ? '\t' : _next == 'n' ? '\n' : _next;
		}
		return _out;
	}

	// Split on the separator; escaped characters never are one ('|' and tabs are always escaped inside a field).
	std::vector<std::string> Fields(const std::string& _line, const char _separator)
	{
		std::vector<std::string> _fields(1);
		for (const char _c : _line)
		{
			if (_c == _separator) _fields.emplace_back();
			else _fields.back() += _c;
		}
		return _fields;
	}

	int ToInt(const std::string& _text)
	{
		int _value = 0;
		std::from_chars(_text.data(), _text.data() + _text.size(), _value);
		return _value;
	}

	struct Cache
	{
		std::unordered_map<std::string, std::shared_ptr<const BlueprintAsset>> assets;
	};

	Cache& GetCache()
	{
		static Cache _cache;
		return _cache;
	}
}

BlueprintNode* BlueprintAsset::FindNode(const int& _id)
{
	for (BlueprintNode& _node : nodes) if (_node.id == _id) return &_node;
	return nullptr;
}

const BlueprintNode* BlueprintAsset::FindNode(const int& _id) const
{
	for (const BlueprintNode& _node : nodes) if (_node.id == _id) return &_node;
	return nullptr;
}

const BlueprintVariable* BlueprintAsset::FindVariable(const std::string& _name) const
{
	for (const BlueprintVariable& _variable : variables) if (_variable.name == _name) return &_variable;
	return nullptr;
}

std::string BlueprintAsset::Encode(const BlueprintAsset& _asset)
{
	std::ostringstream _out;
	_out << "KBP 1\n";
	for (const BlueprintVariable& _variable : _asset.variables)
		_out << "V|" << Escape(_variable.name) << '|' << _variable.type << '|' << Escape(_variable.value) << '\n';
	for (const BlueprintNode& _node : _asset.nodes)
	{
		_out << "N|" << _node.id << '|' << Escape(_node.type) << '|' << _node.position.x << '|' << _node.position.y << '|';
		bool _first = true;
		for (const auto& [_pin, _value] : _node.values)
		{
			if (!_first) _out << '\t';
			_out << Escape(_pin) << '\t' << Escape(_value);
			_first = false;
		}
		_out << '\n';
	}
	for (const BlueprintLink& _link : _asset.links)
		_out << "L|" << _link.fromNode << '|' << Escape(_link.fromPin) << '|' << _link.toNode << '|' << Escape(_link.toPin) << '\n';
	return _out.str();
}

BlueprintAsset BlueprintAsset::Decode(const std::string& _text)
{
	BlueprintAsset _asset;
	std::istringstream _in(_text);
	std::string _line;
	while (std::getline(_in, _line))
	{
		if (!_line.empty() && _line.back() == '\r') _line.pop_back();
		const std::vector<std::string> _f = Fields(_line, '|');
		if (_f[0] == "V" && _f.size() >= 4)
			_asset.variables.push_back({ Unescape(_f[1]), std::clamp(ToInt(_f[2]), 1, 7), Unescape(_f[3]) });
		else if (_f[0] == "N" && _f.size() >= 6)
		{
			BlueprintNode _node;
			_node.id = ToInt(_f[1]);
			_node.type = Unescape(_f[2]);
			_node.position = FVector2(std::get<float>(BlueprintValues::Parse(_f[3], PinType::Float)), std::get<float>(BlueprintValues::Parse(_f[4], PinType::Float)));
			if (!_f[5].empty())
			{
				const std::vector<std::string> _pairs = Fields(_f[5], '\t');
				for (size_t _i = 0; _i + 1 < _pairs.size(); _i += 2)
					_node.values[Unescape(_pairs[_i])] = Unescape(_pairs[_i + 1]);
			}
			_asset.nextId = std::max(_asset.nextId, _node.id + 1);
			_asset.nodes.push_back(std::move(_node));
		}
		else if (_f[0] == "L" && _f.size() >= 5)
			_asset.links.push_back({ ToInt(_f[1]), Unescape(_f[2]), ToInt(_f[3]), Unescape(_f[4]) });
	}
	return _asset;
}

std::filesystem::path BlueprintAsset::Folder()
{
	return std::filesystem::path("Content") / "Blueprints";
}

std::filesystem::path BlueprintAsset::PathOf(const std::string& _name)
{
	return Folder() / (_name + EXTENSION);
}

bool BlueprintAsset::Exists(const std::string& _name)
{
	return !_name.empty() && AssetPack::Exists(PathOf(_name));
}

std::shared_ptr<const BlueprintAsset> BlueprintAsset::Load(const std::string& _name)
{
	if (_name.empty()) return nullptr;
	Cache& _cache = GetCache();
	const auto _found = _cache.assets.find(_name);
	if (_found != _cache.assets.end()) return _found->second;

	std::unique_ptr<std::istream> _file = AssetPack::OpenStream(PathOf(_name));	// a packed game has its files in its exe
	if (!_file || !*_file)
	{
		LOG_WARNING("Blueprint: no file " + PathOf(_name).generic_string());
		return nullptr;
	}
	std::ostringstream _text;
	_text << _file->rdbuf();
	auto _asset = std::make_shared<const BlueprintAsset>(Decode(_text.str()));
	_cache.assets[_name] = _asset;
	return _asset;
}

bool BlueprintAsset::Save(const std::string& _name, const BlueprintAsset& _asset)
{
	std::error_code _error;
	std::filesystem::create_directories(PathOf(_name).parent_path(), _error);
	std::ofstream _file(PathOf(_name), std::ios::trunc | std::ios::binary);
	if (!_file.is_open()) return false;
	_file << Encode(_asset);
	_file.close();
	GetCache().assets[_name] = std::make_shared<const BlueprintAsset>(_asset);
	const std::string _copy = _name;	// listeners may change the cache
	OnChanged().Broadcast(_copy);
	return true;
}

std::vector<std::string> BlueprintAsset::List()
{
	std::vector<std::string> _names;
	std::error_code _error;
	if (!std::filesystem::is_directory(Folder(), _error)) return _names;
	for (const auto& _entry : std::filesystem::recursive_directory_iterator(Folder(), _error))
	{
		if (!_entry.is_regular_file() || _entry.path().extension() != EXTENSION) continue;
		std::filesystem::path _relative = std::filesystem::relative(_entry.path(), Folder(), _error);
		_relative.replace_extension();
		_names.push_back(_relative.generic_string());
	}
	std::sort(_names.begin(), _names.end());
	return _names;
}

Event<const std::string&>& BlueprintAsset::OnChanged()
{
	static Event<const std::string&> _event;
	return _event;
}
