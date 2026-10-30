#include "ScriptCreator.h"
#include "Reflection/Reflection.h"
#include <set>

using namespace Krampus;

// New scripts (Content panel > + New Script): a class of any registered parent, the engine's or the game's, with a name that cannot
// clash with anything that exists (a clash would only show as compile errors, or as a class that silently replaces another).

namespace
{
	// ---- What the sources declare: every type name, and where the classes are defined (to include their header).

	struct Declarations
	{
		std::map<std::string, std::string>		types;			// name -> file that declares it (for the error message)
		std::map<std::string, std::string>		includes;		// class -> header to include, relative to its include folder
		std::set<std::string>					scriptClasses;	// classes defined in Content/Scripts
	};

	bool g_declarationsStale = true;

	bool IsWordCharacter(const char _c)
	{
		return std::isalnum(static_cast<unsigned char>(_c)) || _c == '_';
	}

	/// <summary>
	/// The names that a header declares (class, struct, union, enum, enum class, using X =), and those it defines (class or struct
	/// followed by : or {). A small word scanner: std::regex takes seconds on the engine's headers in Debug.
	/// </summary>
	void ScanHeader(const std::string& _text, std::vector<std::string>& _declared, std::vector<std::string>& _defined)
	{
		const size_t _size = _text.size();
		const auto _skipSpaces = [&](size_t _i) { while (_i < _size && std::isspace(static_cast<unsigned char>(_text[_i]))) ++_i; return _i; };
		const auto _readWord = [&](size_t& _i) { const size_t _start = _i; while (_i < _size && IsWordCharacter(_text[_i])) ++_i; return _text.substr(_start, _i - _start); };

		for (size_t _i = 0; _i < _size; )
		{
			if (!IsWordCharacter(_text[_i])) { ++_i; continue; }
			const bool _startOfWord = _i == 0 || !IsWordCharacter(_text[_i - 1]);
			std::string _word = _readWord(_i);
			if (!_startOfWord) continue;

			const bool _record = _word == "class" || _word == "struct" || _word == "union";
			if (!_record && _word != "enum" && _word != "using") continue;

			size_t _j = _skipSpaces(_i);
			std::string _name = _readWord(_j);
			if (_word == "enum" && (_name == "class" || _name == "struct")) { _j = _skipSpaces(_j); _name = _readWord(_j); }
			if (_name.empty() || std::isdigit(static_cast<unsigned char>(_name[0]))) continue;

			_j = _skipSpaces(_j);
			if (_word == "using")
			{
				if (_j < _size && _text[_j] == '=') _declared.push_back(_name);
				continue;
			}
			_declared.push_back(_name);
			if (_name == "final") continue;
			if (_record)
			{
				size_t _k = _j;
				if (_text.compare(_k, 5, "final") == 0) _k = _skipSpaces(_k + 5);
				if (_k < _size && (_text[_k] == ':' || _text[_k] == '{') && _text.compare(_k, 2, "::") != 0) _defined.push_back(_name);
			}
		}
	}

	const Declarations& ScanDeclarations()
	{
		// Read once, then again only when asked (the New Script window opens, a script is renamed): the name is checked at each key typed.
		static Declarations _declarations;
		if (!g_declarationsStale) return _declarations;
		g_declarationsStale = false;
		_declarations = Declarations();

		// Folder, the folder includes are relative to, and whether the classes there are scripts.
		const std::vector<std::pair<std::filesystem::path, bool>> _roots = {
			{ "Engine/Source", false }, { "Game/Source", false }, { SCRIPTS_DIRECTORY, true } };
		for (const auto& [_root, _scripts] : _roots)
		{
			std::error_code _error;
			for (const auto& _entry : std::filesystem::recursive_directory_iterator(_root, _error))
			{
				const std::string _extension = _entry.path().extension().string();
				if (!_entry.is_regular_file() || (_extension != ".h" && _extension != ".hpp")) continue;

				std::ifstream _file(_entry.path());
				const std::string _text((std::istreambuf_iterator<char>(_file)), std::istreambuf_iterator<char>());
				const std::string _relative = std::filesystem::relative(_entry.path(), _root, _error).generic_string();

				std::vector<std::string> _declared, _defined;
				ScanHeader(_text, _declared, _defined);
				for (const std::string& _name : _declared)
					_declarations.types.emplace(_name, _entry.path().generic_string());
				for (const std::string& _name : _defined)
				{
					_declarations.includes.emplace(_name, _relative);
					if (_scripts) _declarations.scriptClasses.insert(_name);
				}
			}
		}
		return _declarations;
	}

	bool IsIdentifier(const std::string& _name)
	{
		if (_name.empty() || std::isdigit(static_cast<unsigned char>(_name[0]))) return false;
		return std::all_of(_name.begin(), _name.end(), [](char _c) { return std::isalnum(static_cast<unsigned char>(_c)) || _c == '_'; });
	}

	bool IsKeyword(const std::string& _name)
	{
		static const std::set<std::string> KEYWORDS = {
			"alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break", "case", "catch", "char", "char8_t",
			"char16_t", "char32_t", "class", "compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast", "continue",
			"co_await", "co_return", "co_yield", "decltype", "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit",
			"export", "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", "new",
			"noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", "private", "protected", "public", "register",
			"reinterpret_cast", "requires", "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
			"switch", "template", "this", "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", "unsigned",
			"using", "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq", "final", "override", "Krampus", "std", "sf" };
		return KEYWORDS.contains(_name);
	}

	/// <returns>The root class of _name (Actor, Component, Widget, WidgetComponent), false when it has none (a settings class...).</returns>
	bool BaseOf(const std::string& _name, ScriptBase& _base, bool& _diegetic)
	{
		_diegetic = false;
		std::set<std::string> _seen;
		for (const ClassInfo* _class = ClassRegistry::Find(_name); _class && _seen.insert(_class->name).second;
			_class = _class->parentName.empty() ? nullptr : ClassRegistry::Find(_class->parentName))
		{
			if (_class->name == "DiegeticWidget") _diegetic = true;
			if (_class->name == "Actor")			{ _base = ScriptBase::Actor; return true; }
			if (_class->name == "Component")		{ _base = ScriptBase::Component; return true; }
			if (_class->name == "Widget")			{ _base = ScriptBase::Widget; return true; }
			if (_class->name == "WidgetComponent")	{ _base = ScriptBase::WidgetComponent; return true; }
		}
		return false;
	}

	/// <summary>The engine classes whose constructor needs more than the owner (or the level).</summary>
	std::string ExtraArgumentsOf(const std::string& _name)
	{
		static const std::map<std::string, std::string> EXTRA = {
			{ "SpriteComponent", "RectangleShapeData()" },
			{ "LabelComponent", "TextData()" },
			{ "PhysicsComponent", "FVector2(100.f, 100.f)" },
			{ "ImageWidgetComponent", "RectangleShapeData()" },
			{ "LabelWidgetComponent", "TextData()" },
			{ "ButtonWidget", "RectangleShapeData()" } };
		const auto _found = EXTRA.find(_name);
		return _found == EXTRA.end() ? std::string() : _found->second;
	}

	const char* MacroOf(const ScriptBase& _base)
	{
		switch (_base)
		{
		case ScriptBase::Component:			return "KCOMPONENT";
		case ScriptBase::Widget:			return "KWIDGET";
		case ScriptBase::WidgetComponent:	return "KWIDGETCOMPONENT";
		default:							return "KACTOR";
		}
	}

	bool WriteText(const std::filesystem::path& _path, const std::string& _text)
	{
		std::ofstream _file(_path, std::ios::trunc);
		_file << _text;
		return _file.good();
	}
}

const char* Krampus::NameOfBase(const ScriptBase& _base)
{
	switch (_base)
	{
	case ScriptBase::Component:			return "Components";
	case ScriptBase::Widget:			return "HUD widgets";
	case ScriptBase::WidgetComponent:	return "Widget components";
	default:							return "Actors";
	}
}

std::vector<ScriptParent> Krampus::ScriptParents()
{
	const Declarations& _declarations = ScanDeclarations();
	std::vector<ScriptParent> _parents;
	std::set<std::string> _added;

	const auto _add = [&](const std::string& _name)
	{
		if (!_added.insert(_name).second) return;
		ScriptParent _parent;
		_parent.name = _name;
		if (!BaseOf(_name, _parent.base, _parent.isDiegetic)) return;
		if (const auto _found = _declarations.includes.find(_name); _found != _declarations.includes.end()) _parent.include = _found->second;
		_parent.isScript = _declarations.scriptClasses.contains(_name);
		_parent.extraArguments = ExtraArgumentsOf(_name);
		_parents.push_back(_parent);
	};

	for (const char* _base : { "Actor", "DiegeticWidget", "Component", "Widget", "WidgetComponent" }) _add(_base);
	for (const ClassInfo* _class : ClassRegistry::GetActorClasses()) _add(_class->name);
	for (const ClassInfo* _class : ClassRegistry::GetComponentClasses()) _add(_class->name);
	for (const ClassInfo* _class : ClassRegistry::GetWidgetClasses()) _add(_class->name);
	for (const ClassInfo* _class : ClassRegistry::GetWidgetComponentClasses()) _add(_class->name);
	return _parents;
}

std::string Krampus::CheckNewClassName(const std::string& _name)
{
	if (_name.empty()) return "Type a name";
	if (!IsIdentifier(_name)) return "Letters, digits and _ only, not starting with a digit";
	if (_name.size() > 64) return "This name is too long";
	if (IsKeyword(_name)) return "\"" + _name + "\" is a word of C++ or of the engine";
	if (_name.find("__") != std::string::npos || (_name[0] == '_' && _name.size() > 1 && std::isupper(static_cast<unsigned char>(_name[1]))))
		return "Names with __ or starting with _ and a capital are reserved by C++";

	if (const ClassInfo* _class = ClassRegistry::Find(_name))
		return "A class named " + _class->name + " already exists";

	const Declarations& _declarations = ScanDeclarations();
	if (const auto _found = _declarations.types.find(_name); _found != _declarations.types.end())
		return _name + " already exists (" + _found->second + ")";

	// A script file of this name anywhere in Content/Scripts, even if it does not declare the class (yet).
	std::error_code _error;
	for (const auto& _entry : std::filesystem::recursive_directory_iterator(SCRIPTS_DIRECTORY, _error))
		if (_entry.is_regular_file() && _entry.path().stem() == _name && (_entry.path().extension() == ".h" || _entry.path().extension() == ".cpp"))
			return "There is already a file " + _entry.path().generic_string();
	return "";
}

std::string Krampus::CreateScript(const std::string& _name, const ScriptParent& _parent, const std::filesystem::path& _folder)
{
	if (const std::string _problem = CheckNewClassName(_name); !_problem.empty()) return _problem;

	// Only in Content/Scripts: the game builds the scripts of that folder.
	std::error_code _error;
	const std::filesystem::path _scripts = std::filesystem::weakly_canonical(SCRIPTS_DIRECTORY, _error);
	const std::filesystem::path _target = std::filesystem::weakly_canonical(_folder, _error);
	const std::filesystem::path _inside = std::filesystem::relative(_target, _scripts, _error);
	const std::filesystem::path _directory = (_error || _inside.empty() || _inside.native().starts_with(L"..")) ? SCRIPTS_DIRECTORY : _folder;
	std::filesystem::create_directories(_directory, _error);

	const std::filesystem::path _header = _directory / (_name + ".h");
	const std::filesystem::path _source = _directory / (_name + ".cpp");
	if (std::filesystem::exists(_header) || std::filesystem::exists(_source))
		return "A script named " + _name + " already exists";

	const std::string& _p = _parent.name;
	const std::string _extra = _parent.extraArguments.empty() ? std::string() : ", " + _parent.extraArguments;
	std::string _includes = "#include \"Reflection/Reflection.h\"\n";
	if (!_parent.include.empty()) _includes += "#include \"" + _parent.include + "\"\n";

	std::string _owner, _ownerType, _comment, _members, _bodies;
	switch (_parent.base)
	{
	case ScriptBase::Actor:
		_owner = "_level";
		_ownerType = "Level*";
		_comment = _parent.isDiegetic
			? "\t// A diegetic widget: an actor kept at a place of the screen. Add it with Add Actor, or in code:\n"
			  "\t// GetLevel()->SpawnDiegeticWidget<" + _name + ">(FVector2(100.f, 100.f));\n"
			: "\t// Add it to a scene with Add Actor, or in code: GetLevel()->SpawnActor<" + _name + ">();\n";
		_members = "\t\tvoid BeginPlay() override;\n\t\tvoid Tick(const Float& _deltaTime) override;\n";
		_bodies = "void " + _name + "::BeginPlay()\n{\n\t" + _p + "::BeginPlay();\n}\n\n"
			"void " + _name + "::Tick(const Float& _deltaTime)\n{\n\t" + _p + "::Tick(_deltaTime);\n}\n\n";
		break;
	case ScriptBase::Component:
		_owner = "_owner";
		_ownerType = "Actor*";
		_comment = "\t// Add it from the Inspector (Add Component) or from an actor constructor with CreateComponent<" + _name + ">().\n";
		_members = "\t\tvoid Tick(const Float& _deltaTime) override;\n";
		_bodies = "void " + _name + "::Tick(const Float& _deltaTime)\n{\n\t" + _p + "::Tick(_deltaTime);\n}\n\n";
		break;
	case ScriptBase::Widget:
		_owner = "_level";
		_ownerType = "Level*";
		_comment = "\t// A HUD widget: add it with Add Widget, or in code: GetLevel()->SpawnWidget<" + _name + ">();\n";
		_members = "\t\tvoid BeginPlay() override;\n\t\tvoid Tick(const Float& _deltaTime) override;\n";
		_bodies = "void " + _name + "::BeginPlay()\n{\n\t" + _p + "::BeginPlay();\n}\n\n"
			"void " + _name + "::Tick(const Float& _deltaTime)\n{\n\t" + _p + "::Tick(_deltaTime);\n}\n\n";
		break;
	case ScriptBase::WidgetComponent:
		_owner = "_owner";
		_ownerType = "Widget*";
		_comment = "\t// Add it to a HUD widget from the Inspector, or in code: _widget->AddComponent<" + _name + ">();\n";
		_members = "\t\tvoid Tick(const Float& _deltaTime) override;\n";
		_bodies = "void " + _name + "::Tick(const Float& _deltaTime)\n{\n\t" + _p + "::Tick(_deltaTime);\n}\n\n";
		break;
	}

	const std::string _h = "#pragma once\n" + _includes + "\nnamespace Krampus\n{\n\n" + _comment
		+ "\tclass " + _name + " : public " + _p + "\n\t{\n\t\t" + MacroOf(_parent.base) + "(" + _name + ", " + _p + ")\n\n\tpublic:\n"
		+ "\t\t" + _name + "(" + _ownerType + " " + _owner + ");\n\n"
		+ "\t\t// Shown in the Inspector and saved in scenes.\n\t\tKPROPERTY(Float, speed) = 100.f;\n\n"
		+ _members + "\t};\n\n}\n";
	const std::string _c = "#include \"" + _name + ".h\"\n\nnamespace Krampus\n{\n\n"
		+ _name + "::" + _name + "(" + _ownerType + " " + _owner + ")\n\t: " + _p + "(" + _owner + _extra + ")\n{\n\tname = NAME_OF(" + _name + ");\n}\n\n"
		+ _bodies + "}\n";

	if (!WriteText(_header, _h) || !WriteText(_source, _c))
	{
		std::filesystem::remove(_header, _error);
		std::filesystem::remove(_source, _error);
		return "Cannot write the script files";
	}

	RefreshClassNames();	// the new class is taken now
	RefreshSolution();	// so that the new class shows up in Visual Studio
	LOG_MSG("Script created: " + _header.generic_string() + " (a " + _p + "). Double-click it to open Visual Studio, then Compile to load it.");
	return "";
}

void Krampus::RefreshClassNames()
{
	g_declarationsStale = true;
}
