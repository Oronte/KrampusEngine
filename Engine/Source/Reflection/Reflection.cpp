#include "Reflection.h"
#include <charconv>
#include <typeindex>

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

	// Mistakes found while the classes register themselves (before main: the Logger does not run yet), logged later.
	std::vector<std::string>& Problems()
	{
		static std::vector<std::string> _problems;
		return _problems;
	}

	// The classes of the engine are described once, the first time the registry is read.
	void EnsureEngineClasses()
	{
		static bool _done = false;
		if (_done) return;
		_done = true;
		RegisterEngineClasses();
		for (const std::string& _problem : Problems())
			LOG_ERROR("Reflection: " + _problem);
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

void ClassInfo::Build() const
{
	if (built) return;
	built = true;
	for (void(*_declare)(ClassInfo&) : declarers)
		_declare(const_cast<ClassInfo&>(*this));
}

// The chain from the oldest parent to this class. A loop in the parents (A's parent is B, B's parent is A) is cut instead of
// recursing forever.
static std::vector<const ClassInfo*> Lineage(const ClassInfo& _class)
{
	std::vector<const ClassInfo*> _chain;
	for (const ClassInfo* _current = &_class; _current && _chain.size() < 64; )
	{
		if (std::find(_chain.begin(), _chain.end(), _current) != _chain.end())
		{
			LOG_ERROR("Reflection: the parents of " + _class.name + " make a loop (" + _current->name + "): check its KCLASS / KACTOR / KCOMPONENT");
			break;
		}
		_chain.push_back(_current);
		_current = _current->parentName.empty() ? nullptr : ClassRegistry::Find(_current->parentName);
	}
	std::reverse(_chain.begin(), _chain.end());
	return _chain;
}

std::vector<const FunctionInfo*> ClassInfo::GetFunctions() const
{
	std::vector<const FunctionInfo*> _result;
	for (const ClassInfo* _class : Lineage(*this))
	{
		_class->Build();
		for (const FunctionInfo& _function : _class->functions)
			_result.push_back(&_function);
	}
	return _result;
}

std::vector<const PropertyInfo*> ClassInfo::GetProperties() const
{
	std::vector<const PropertyInfo*> _result;
	for (const ClassInfo* _class : Lineage(*this))
	{
		_class->Build();
		for (const PropertyInfo& _property : _class->properties)
			_result.push_back(&_property);
	}
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
	const std::string _parent = StripNamespace(_parentName);
	if (_parent == StripNamespace(_name))
	{
		Problems().push_back("class " + std::string(_name) + " is declared as its own parent");
		return;
	}
	ClassRegistry::FindOrAdd(_name).parentName = _parent;
}

PropertyRegistrar::PropertyRegistrar(const char* _className, void(*_declare)(ClassInfo&))
{
	ClassRegistry::FindOrAdd(_className).AddDeclarer(_declare);
}

// Two classes of the same name (two scripts, or a script named like a class of the engine): the second replaces the first, which
// can no longer be created by name. Said in the Console when the registry is first used (the Logger is not running yet here).
static void NoteTwice(const bool _alreadySet, const char* _className)
{
	if (_alreadySet)
		Problems().push_back("two classes are named " + std::string(_className) + ": rename one of them (only one can be created by name)");
}

ActorRegistrar::ActorRegistrar(const char* _className, Actor*(*_spawn)(Level*))
{
	ClassInfo& _info = ClassRegistry::FindOrAdd(_className);
	NoteTwice(static_cast<bool>(_info.factory), _className);
	_info.factory = _spawn;
}

ComponentRegistrar::ComponentRegistrar(const char* _className, Component*(*_add)(Actor*))
{
	ClassInfo& _info = ClassRegistry::FindOrAdd(_className);
	NoteTwice(static_cast<bool>(_info.componentFactory), _className);
	_info.componentFactory = _add;
}

WidgetRegistrar::WidgetRegistrar(const char* _className, Widget*(*_spawn)(Level*))
{
	ClassInfo& _info = ClassRegistry::FindOrAdd(_className);
	NoteTwice(static_cast<bool>(_info.widgetFactory), _className);
	_info.widgetFactory = _spawn;
}

WidgetComponentRegistrar::WidgetComponentRegistrar(const char* _className, WidgetComponent*(*_add)(Widget*))
{
	ClassInfo& _info = ClassRegistry::FindOrAdd(_className);
	NoteTwice(static_cast<bool>(_info.widgetComponentFactory), _className);
	_info.widgetComponentFactory = _add;
}

// ---- References to actors and components (KPROPERTY(Actor*, ...))

std::string Reflection::TypeName(const std::type_info& _type)
{
	return StripNamespace(_type.name());
}

std::string Reflection::EncodeReference(IGameObject* _target)
{
	if (const Actor* _actor = dynamic_cast<const Actor*>(_target))
		return std::format("{:016x}", _actor->GetId());
	if (const Component* _component = dynamic_cast<const Component*>(_target); _component && _component->GetOwner())
		return std::format("{:016x}/{}", _component->GetOwner()->GetId(), ClassNameOf(*_component));
	return "";
}

namespace
{
	Level* LevelOf(IGameObject* _object)
	{
		if (Actor* _actor = dynamic_cast<Actor*>(_object))							return _actor->GetLevel();
		if (Component* _component = dynamic_cast<Component*>(_object))				return _component->GetLevel();
		if (Widget* _widget = dynamic_cast<Widget*>(_object))						return _widget->GetLevel();
		if (WidgetComponent* _component = dynamic_cast<WidgetComponent*>(_object))	return _component->GetOwner() ? _component->GetOwner()->GetLevel() : nullptr;
		return nullptr;
	}

	// The reference properties of the class of _object (cached by C++ type: this runs each time an actor is destroyed).
	const std::vector<const PropertyInfo*>& ReferencesOf(const IGameObject& _object)
	{
		static std::unordered_map<std::type_index, std::vector<const PropertyInfo*>> _cache;
		const auto _found = _cache.find(std::type_index(typeid(_object)));
		if (_found != _cache.end()) return _found->second;

		std::vector<const PropertyInfo*> _references;
		if (const ClassInfo* _class = ClassRegistry::Find(_object.ClassName()))
			for (const PropertyInfo* _property : _class->GetProperties())
				if (_property->IsReference() && _property->getObject && _property->setObject)
					_references.push_back(_property);
		return _cache.emplace(std::type_index(typeid(_object)), std::move(_references)).first->second;
	}

	void Forget(IGameObject& _holder, IGameObject* _object, const Actor* _actor)
	{
		for (const PropertyInfo* _property : ReferencesOf(_holder))
		{
			IGameObject* _target = _property->getObject(&_holder);
			if (!_target) continue;
			const Component* _component = _actor ? dynamic_cast<const Component*>(_target) : nullptr;
			if (_target == _object || (_component && _component->GetOwner() == _actor))
				_property->setObject(&_holder, nullptr);
		}
	}
}

IGameObject* Reflection::ResolveReference(IGameObject* _context, const std::string& _text)
{
	if (_text.empty()) return nullptr;
	Level* _level = LevelOf(_context);
	if (!_level) return nullptr;

	const size_t _slash = _text.find('/');
	std::uint64_t _id = 0;
	const std::string _idText = _text.substr(0, _slash);
	if (std::from_chars(_idText.data(), _idText.data() + _idText.size(), _id, 16).ec != std::errc()) return nullptr;

	for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
	{
		if (_actor->GetId() != _id) continue;
		if (_slash == std::string::npos) return _actor.get();
		const std::string _className = _text.substr(_slash + 1);
		for (const std::unique_ptr<Component>& _component : _actor->GetComponents())
			if (ClassNameOf(*_component) == _className) return _component.get();
		return nullptr;
	}
	return nullptr;
}

void Krampus::ForgetReferencesTo(Level* _level, IGameObject* _object)
{
	if (!_level || !_object) return;
	const Actor* _actor = dynamic_cast<const Actor*>(_object);	// its components go with it
	for (const std::unique_ptr<Actor>& _holder : _level->GetActorManager().GetActors())
	{
		Forget(*_holder, _object, _actor);
		for (const std::unique_ptr<Component>& _component : _holder->GetComponents())
			Forget(*_component, _object, _actor);
	}
	for (const std::unique_ptr<Widget>& _widget : _level->GetHUD().GetWidgets())
	{
		Forget(*_widget, _object, _actor);
		for (const std::unique_ptr<WidgetComponent>& _component : _widget->GetComponents())
			Forget(*_component, _object, _actor);
	}
}
