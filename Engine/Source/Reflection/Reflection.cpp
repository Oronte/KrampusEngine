#include "Reflection.h"

using namespace Krampus;

namespace
{
	std::unordered_map<std::string, std::unique_ptr<ClassInfo>>& Classes()
	{
		static std::unordered_map<std::string, std::unique_ptr<ClassInfo>> _classes;
		return _classes;
	}

	// "class Krampus::Exemple::Enemy" -> "Enemy"
	std::string StripNamespace(const std::string& _name)
	{
		const size_t _position = _name.rfind("::");
		std::string _result = _position == std::string::npos ? _name : _name.substr(_position + 2);

		for (const char* _prefix : { "class ", "struct " })
			if (_result.starts_with(_prefix))
				_result = _result.substr(std::strlen(_prefix));
		return _result;
	}

	std::vector<ClassInfo*> Collect(const std::function<bool(const ClassInfo&)>& _filter);

	// The classes of the engine are described once, the first time the registry is read.
	void EnsureEngineClasses()
	{
		static bool _done = false;
		if (_done) return;
		_done = true;
		RegisterEngineClasses();
		LOG(VerbosityType::Log, "Reflection: " + std::to_string(Classes().size()) + " classes registered (" + std::to_string(Collect([](const ClassInfo& _i) { return static_cast<bool>(_i.factory); }).size()) + " actors, "
			+ std::to_string(Collect([](const ClassInfo& _i) { return static_cast<bool>(_i.componentFactory); }).size()) + " components, "
			+ std::to_string(Collect([](const ClassInfo& _i) { return static_cast<bool>(_i.widgetFactory); }).size()) + " widgets)");
	}

	std::vector<ClassInfo*> Collect(const std::function<bool(const ClassInfo&)>& _filter)
	{
		EnsureEngineClasses();

		std::vector<ClassInfo*> _result;
		for (const auto& [_name, _info] : Classes())
			if (_filter(*_info))
				_result.push_back(_info.get());

		std::sort(_result.begin(), _result.end(), [](const ClassInfo* _a, const ClassInfo* _b) { return _a->name < _b->name; });
		return _result;
	}
}

std::vector<const PropertyInfo*> ClassInfo::GetProperties() const
{
	std::vector<const PropertyInfo*> _result;
	if (!parentName.empty())
		if (const ClassInfo* _parent = ClassRegistry::Find(parentName))
			_result = _parent->GetProperties();

	if (!built)
	{
		built = true;
		for (void(*_declare)(ClassInfo&) : declarers)
			_declare(const_cast<ClassInfo&>(*this));
	}

	for (const PropertyInfo& _property : properties)
		_result.push_back(&_property);
	return _result;
}

ClassInfo& ClassRegistry::FindOrAdd(const std::string& _name)
{
	const std::string _key = StripNamespace(_name);
	std::unique_ptr<ClassInfo>& _slot = Classes()[_key];
	if (!_slot)
	{
		_slot = std::make_unique<ClassInfo>();
		_slot->name = _key;
	}
	return *_slot;
}

ClassInfo* ClassRegistry::Find(const std::string& _name)
{
	EnsureEngineClasses();

	const auto _it = Classes().find(StripNamespace(_name));
	return _it == Classes().end() ? nullptr : _it->second.get();
}

std::vector<ClassInfo*> ClassRegistry::GetActorClasses()
{
	return Collect([](const ClassInfo& _info) { return static_cast<bool>(_info.factory); });
}

std::vector<ClassInfo*> ClassRegistry::GetComponentClasses()
{
	return Collect([](const ClassInfo& _info) { return static_cast<bool>(_info.componentFactory); });
}

std::vector<ClassInfo*> ClassRegistry::GetWidgetClasses()
{
	return Collect([](const ClassInfo& _info) { return static_cast<bool>(_info.widgetFactory); });
}

std::vector<ClassInfo*> ClassRegistry::GetWidgetComponentClasses()
{
	return Collect([](const ClassInfo& _info) { return static_cast<bool>(_info.widgetComponentFactory); });
}

std::string Krampus::ClassNameOf(const IGameObject& _object)
{
	return StripNamespace(_object.ClassName());
}

ClassRegistrar::ClassRegistrar(const char* _name, const char* _parentName)
{
	ClassRegistry::FindOrAdd(_name).parentName = StripNamespace(_parentName);
}

PropertyRegistrar::PropertyRegistrar(const char* _className, void(*_declare)(ClassInfo&))
{
	ClassRegistry::FindOrAdd(_className).AddDeclarer(_declare);
}

ActorRegistrar::ActorRegistrar(const char* _className, Actor*(*_spawn)(Level*))
{
	ClassRegistry::FindOrAdd(_className).factory = _spawn;
}

ComponentRegistrar::ComponentRegistrar(const char* _className, Component*(*_add)(Actor*))
{
	ClassRegistry::FindOrAdd(_className).componentFactory = _add;
}
