#include "ShapeAssets.h"
#include "Core/AssetPack.h"
#include <unordered_map>
#include <set>

using namespace Krampus;

namespace
{
	std::unordered_map<std::string, Polygon2D>& Cache()
	{
		static std::unordered_map<std::string, Polygon2D> _cache;
		return _cache;
	}

	std::set<std::string>& Warned()
	{
		static std::set<std::string> _warned;
		return _warned;
	}

	const Polygon2D& Fallback()
	{
		static const Polygon2D _hexagon = Polygon2D::Regular(6, 50.f);
		return _hexagon;
	}

	std::string Trim(const std::string& _text)
	{
		const size_t _begin = _text.find_first_not_of(" \t\r\n");
		if (_begin == std::string::npos) return std::string();
		return _text.substr(_begin, _text.find_last_not_of(" \t\r\n") - _begin + 1);
	}

	bool Read(const std::string& _name, Polygon2D& _out)
	{
		std::unique_ptr<std::istream> _file = AssetPack::OpenStream(ShapeAssets::PathOf(_name));
		if (!_file) return false;

		std::string _line;
		while (std::getline(*_file, _line))
		{
			_line = Trim(_line);
			if (_line.empty() || _line[0] == '#') continue;
			const size_t _equal = _line.find('=');
			if (_equal == std::string::npos || Trim(_line.substr(0, _equal)) != "points") continue;
			_out = Polygon2D::Decode(_line.substr(_equal + 1));
		}
		return _out.IsValid();
	}
}

std::filesystem::path ShapeAssets::Folder()
{
	return std::filesystem::path(CONTENT_PATH) / "Shapes";
}

std::filesystem::path ShapeAssets::PathOf(const std::string& _name)
{
	return Folder() / (_name + EXTENSION);
}

const Polygon2D& ShapeAssets::Get(const std::string& _name)
{
	if (const auto _found = Cache().find(_name); _found != Cache().end())
		return _found->second;

	Polygon2D _polygon;
	if (_name.empty() || !Read(_name, _polygon))
	{
		if (Warned().insert(_name).second)
			LOG_WARNING("Shapes: no valid shape \"" + _name + "\" (" + PathOf(_name).generic_string() + "): a hexagon is used instead");
		return Fallback();
	}
	LOG(VerbosityType::Verbose, "Shapes: loaded " + _name + " (" + std::to_string(_polygon.points.size()) + " points)");
	return Cache()[_name] = std::move(_polygon);
}

bool ShapeAssets::Exists(const std::string& _name)
{
	return !_name.empty() && AssetPack::Exists(PathOf(_name));
}

bool ShapeAssets::Save(const std::string& _name, const Polygon2D& _polygon)
{
	if (_name.empty() || !_polygon.IsValid())
	{
		LOG_ERROR("Shapes: cannot save \"" + _name + "\": a shape needs a name and at least 3 points that are not on one line");
		return false;
	}

	const std::filesystem::path _path = PathOf(_name);
	std::error_code _error;
	std::filesystem::create_directories(_path.parent_path(), _error);
	std::ofstream _file(_path, std::ios::trunc);
	if (!_file)
	{
		LOG_ERROR("Shapes: cannot write " + _path.generic_string());
		return false;
	}
	_file << "# KrampusEngine shape, made in the Shape Editor (Tools > Shape Editor).\n";
	_file << "# Points in pixels around the actor's position (0, 0), in order around the shape. It may be concave; its edges must not cross.\n";
	_file << "points = " << _polygon.Encode() << "\n";
	_file.close();
	if (!_file)
	{
		LOG_ERROR("Shapes: cannot write " + _path.generic_string());
		return false;
	}

	Cache()[_name] = _polygon;
	Warned().erase(_name);
	LOG_MSG("Shapes: saved " + _name + " (" + std::to_string(_polygon.points.size()) + " points)");
	const std::string _copy = _name;
	OnChanged().Broadcast(_copy);
	return true;
}

bool ShapeAssets::Delete(const std::string& _name)
{
	std::error_code _error;
	const bool _removed = std::filesystem::remove(PathOf(_name), _error);
	Cache().erase(_name);
	if (_removed)
	{
		LOG_MSG("Shapes: deleted " + _name);
		const std::string _copy = _name;
		OnChanged().Broadcast(_copy);
	}
	return _removed;
}

bool ShapeAssets::Rename(const std::string& _from, const std::string& _to)
{
	if (_to.empty() || Exists(_to)) return false;
	std::error_code _error;
	std::filesystem::create_directories(PathOf(_to).parent_path(), _error);
	std::filesystem::rename(PathOf(_from), PathOf(_to), _error);
	if (_error) return false;

	Cache().erase(_from);
	LOG_MSG("Shapes: renamed " + _from + " to " + _to + " (the components that used the old name show a hexagon until they are given the new one)");
	const std::string _old = _from, _new = _to;
	OnChanged().Broadcast(_old);
	OnChanged().Broadcast(_new);
	return true;
}

std::vector<std::string> ShapeAssets::List()
{
	std::vector<std::string> _names;
	std::error_code _error;
	for (const std::filesystem::directory_entry& _entry : std::filesystem::recursive_directory_iterator(Folder(), _error))
	{
		if (!_entry.is_regular_file() || _entry.path().extension() != EXTENSION) continue;
		std::filesystem::path _relative = std::filesystem::relative(_entry.path(), Folder(), _error);
		_relative.replace_extension();
		_names.push_back(_relative.generic_string());
	}
	std::sort(_names.begin(), _names.end());
	return _names;
}

void ShapeAssets::ClearCache()
{
	Cache().clear();
	Warned().clear();
}

Event<const std::string&>& ShapeAssets::OnChanged()
{
	static Event<const std::string&> _event;
	return _event;
}

void ShapeAssets::CreateStarterShapes()
{
	if (!List().empty()) return;

	LOG_MSG("Shapes: Content/Shapes is empty, writing a few shapes to start with");
	Save("Triangle", Polygon2D::Regular(3, 60.f));
	Save("Hexagon", Polygon2D::Regular(6, 50.f));
	Save("Octagon", Polygon2D::Regular(8, 50.f, -67.5f));
	Save("Star", Polygon2D::Star(5, 60.f, 26.f));
	Save("Arrow", Polygon2D({ FVector2(-60.f, -15.f), FVector2(10.f, -15.f), FVector2(10.f, -40.f), FVector2(60.f, 0.f),
		FVector2(10.f, 40.f), FVector2(10.f, 15.f), FVector2(-60.f, 15.f) }));
	Save("LShape", Polygon2D({ FVector2(-50.f, -60.f), FVector2(-10.f, -60.f), FVector2(-10.f, 20.f), FVector2(50.f, 20.f),
		FVector2(50.f, 60.f), FVector2(-50.f, 60.f) }));
}
