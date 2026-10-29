#include "Reflection.h"
#include "UI/Widgets/Widget.h"

using namespace Krampus;

namespace
{
	// The list is one string: calls separated by ';', fields of a call by '|'. Those two characters and '%' are escaped inside a field.
	std::string Escape(const std::string& _text)
	{
		std::string _result;
		for (const char _c : _text)
		{
			if (_c == '%') _result += "%25";
			else if (_c == '|') _result += "%7C";
			else if (_c == ';') _result += "%3B";
			else if (_c == 0x0A) _result += "%0A";
			else _result += _c;
		}
		return _result;
	}

	std::string Unescape(const std::string& _text)
	{
		std::string _result;
		for (size_t _i = 0; _i < _text.size(); ++_i)
		{
			if (_text[_i] == '%' && _i + 2 < _text.size())
			{
				const std::string _code = _text.substr(_i + 1, 2);
				if (_code == "25") { _result += '%'; _i += 2; continue; }
				if (_code == "7C") { _result += '|'; _i += 2; continue; }
				if (_code == "3B") { _result += ';'; _i += 2; continue; }
				if (_code == "0A") { _result += static_cast<char>(0x0A); _i += 2; continue; }
			}
			_result += _text[_i];
		}
		return _result;
	}

	std::vector<std::string> Split(const std::string& _text, const char _separator)
	{
		std::vector<std::string> _parts;
		size_t _start = 0;
		for (size_t _end = _text.find(_separator); _end != std::string::npos; _end = _text.find(_separator, _start))
		{
			_parts.push_back(_text.substr(_start, _end - _start));
			_start = _end + 1;
		}
		_parts.push_back(_text.substr(_start));
		return _parts;
	}

	// The first character tells the type: b bool, i int, f float, s string, v vector2, c color.
	std::string EncodeValue(const PropertyValue& _value)
	{
		return std::visit([](const auto& _v) -> std::string
		{
			using V = std::decay_t<decltype(_v)>;
			if constexpr (std::is_same_v<V, bool>)				return std::string("b") + (_v ? "1" : "0");
			else if constexpr (std::is_same_v<V, int>)			return "i" + std::to_string(_v);
			else if constexpr (std::is_same_v<V, float>)		return "f" + std::format("{}", _v);
			else if constexpr (std::is_same_v<V, std::string>)	return "s" + _v;
			else if constexpr (std::is_same_v<V, FVector2>)		return "v" + std::format("{},{}", _v.x, _v.y);
			else												return "c" + std::format("{},{},{},{}", static_cast<int>(_v.r), static_cast<int>(_v.g), static_cast<int>(_v.b), static_cast<int>(_v.a));
		}, _value);
	}

	PropertyValue DecodeValue(const std::string& _text)
	{
		if (_text.empty()) return std::string();
		const std::string _body = _text.substr(1);
		try
		{
			switch (_text[0])
			{
			case 'b': return _body == "1";
			case 'i': return std::stoi(_body);
			case 'f': return std::stof(_body);
			case 'v':
			{
				const std::vector<std::string> _p = Split(_body, ',');
				if (_p.size() == 2) return FVector2(std::stof(_p[0]), std::stof(_p[1]));
				break;
			}
			case 'c':
			{
				const std::vector<std::string> _p = Split(_body, ',');
				if (_p.size() == 4)
					return Color(static_cast<std::uint8_t>(std::stoi(_p[0])), static_cast<std::uint8_t>(std::stoi(_p[1])),
						static_cast<std::uint8_t>(std::stoi(_p[2])), static_cast<std::uint8_t>(std::stoi(_p[3])));
				break;
			}
			default: return _body;
			}
		}
		catch (const std::exception& _exception) { LOG_WARNING(std::string("EventList: unreadable value (") + _exception.what() + ")"); }
		return std::string();
	}

	Level* LevelOf(IGameObject* _object)
	{
		if (Actor* _actor = dynamic_cast<Actor*>(_object)) return _actor->GetLevel<Level>();
		if (Component* _component = dynamic_cast<Component*>(_object)) return _component->GetLevel();
		if (Widget* _widget = dynamic_cast<Widget*>(_object)) return _widget->GetLevel<Level>();
		if (WidgetComponent* _component = dynamic_cast<WidgetComponent*>(_object)) return _component->GetLevel();
		return nullptr;
	}

	// The actor or widget an object is (or belongs to).
	IGameObject* RootOf(IGameObject* _object)
	{
		if (Component* _component = dynamic_cast<Component*>(_object)) return _component->GetOwner();
		if (WidgetComponent* _component = dynamic_cast<WidgetComponent*>(_object)) return _component->GetOwner();
		return _object;
	}

	IGameObject* FindTarget(Level* _level, IGameObject* _owner, const std::string& _target)
	{
		if (_target.empty()) return RootOf(_owner);

		if (_target.starts_with("A:"))
		{
			for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
				if (_actor->GetName().StdString() == _target.substr(2))
					return _actor.get();
		}
		else if (_target.starts_with("W:"))
		{
			for (const std::unique_ptr<Widget>& _widget : _level->GetHUD().GetWidgets())
				if (_widget->GetName().StdString() == _target.substr(2))
					return _widget.get();
		}
		return nullptr;
	}

	template <typename C>
	IGameObject* FindComponent(const std::vector<std::unique_ptr<C>>& _components, const std::string& _className)
	{
		for (const std::unique_ptr<C>& _component : _components)
			if (ClassNameOf(*_component) == _className)
				return _component.get();
		return nullptr;
	}
}

std::string EventList::Encode() const
{
	std::string _text;
	for (const EventCall& _call : calls)
	{
		if (!_text.empty()) _text += ';';
		_text += Escape(_call.target) + "|" + Escape(_call.component) + "|" + Escape(_call.function) + "|" + Escape(EncodeValue(_call.argument));
	}
	return _text;
}

EventList EventList::Decode(const std::string& _text)
{
	EventList _list;
	if (_text.empty()) return _list;

	for (const std::string& _entry : Split(_text, ';'))
	{
		const std::vector<std::string> _fields = Split(_entry, '|');
		if (_fields.size() < 4) continue;

		EventCall _call;
		_call.target = Unescape(_fields[0]);
		_call.component = Unescape(_fields[1]);
		_call.function = Unescape(_fields[2]);
		_call.argument = DecodeValue(Unescape(_fields[3]));
		_list.calls.push_back(std::move(_call));
	}
	return _list;
}

void EventList::Invoke(IGameObject* _owner) const
{
	if (calls.empty() || !_owner) return;

	Level* _level = LevelOf(_owner);
	if (!_level) return;
	Reflection::CurrentEngine() = _level->GetWorld();

	// Copy: a function may change or destroy the object that owns this list.
	const std::vector<EventCall> _calls = calls;
	for (const EventCall& _call : _calls)
	{
		IGameObject* _object = Resolve(_owner, _call);
		if (!_object)
		{
			LOG_WARNING("Event: target \"" + _call.target + "\" " + _call.component + " not found for " + _call.function);
			continue;
		}

		const ClassInfo* _class = ClassRegistry::Find(_call.target == "Game" ? std::string("Game") : ClassNameOf(*_object));
		const FunctionInfo* _function = nullptr;
		if (_class)
			for (const FunctionInfo* _candidate : _class->GetFunctions())
				if (_candidate->name == _call.function)
					_function = _candidate;

		if (!_function)
		{
			LOG_WARNING("Event: no function \"" + _call.function + "\" on " + (_class ? _class->name : std::string("?")));
			continue;
		}

		try
		{
			_function->call(_object, _function->hasArgument ? &_call.argument : nullptr);
		}
		catch (const std::bad_variant_access&)
		{
			LOG_WARNING("Event: the argument of \"" + _call.function + "\" has the wrong type");
		}
	}
}

IGameObject* EventList::Resolve(IGameObject* _owner, const EventCall& _call)
{
	if (_call.target == "Game") return _owner;	// the functions of the class "Game" do not use the object

	Level* _level = LevelOf(_owner);
	if (!_level) return nullptr;

	IGameObject* _root = FindTarget(_level, _owner, _call.target);
	if (!_root || _call.component.empty()) return _root;

	if (Actor* _actor = dynamic_cast<Actor*>(_root)) return FindComponent(_actor->GetComponents(), _call.component);
	if (Widget* _widget = dynamic_cast<Widget*>(_root)) return FindComponent(_widget->GetComponents(), _call.component);
	return nullptr;
}
