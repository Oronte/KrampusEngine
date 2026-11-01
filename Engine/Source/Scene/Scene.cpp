#include "Scene.h"
#include "Components/CameraComponent.h"
#include "Actors/EmptyActor.h"
#include "Actors/CameraActor.h"
#include "Managers/SaveManager.h"
#include "Managers/LevelManager.h"
#include "UI/Widgets/Widget.h"
#include <charconv>

using namespace Krampus;

namespace
{
	constexpr int SCENE_VERSION = 2;

	void WriteValue(SaveNode& _node, const String& _key, const PropertyValue& _value)
	{
		std::visit([&](const auto& _v)
		{
			using V = std::decay_t<decltype(_v)>;
			if constexpr (std::is_same_v<V, bool>)				_node.Set(_key, Bool(_v));
			else if constexpr (std::is_same_v<V, int>)			_node.Set(_key, Int(_v));
			else if constexpr (std::is_same_v<V, float>)		_node.Set(_key, Float(_v));
			else if constexpr (std::is_same_v<V, std::string>)	_node.Set(_key, String(_v));
			else if constexpr (std::is_same_v<V, FVector2>)		_node.Set(_key, std::vector<Float>{ Float(_v.x), Float(_v.y) });
			else												_node.Set(_key, std::vector<Int>{ Int(_v.r), Int(_v.g), Int(_v.b), Int(_v.a) });
		}, _value);
	}

	std::optional<PropertyValue> ReadValue(const SaveNode& _node, const PropertyType& _type, const String& _key)
	{
		switch (_type)
		{
		case PropertyType::Bool:
			if (auto _v = _node.TryGet<Bool>(_key)) return PropertyValue(static_cast<bool>(*_v));
			break;
		case PropertyType::Flags:
		case PropertyType::Enum:
		case PropertyType::Int:
			if (auto _v = _node.TryGet<Int>(_key)) return PropertyValue(static_cast<int>(*_v));
			break;
		case PropertyType::Float:
			if (auto _v = _node.TryGet<Float>(_key)) return PropertyValue(static_cast<float>(*_v));
			break;
		case PropertyType::Texture:
		case PropertyType::Font:
		case PropertyType::Scene:
		case PropertyType::Events:
		case PropertyType::InputBindings:
		case PropertyType::MessageBindings:
		case PropertyType::Shape:
		case PropertyType::Sound:
		case PropertyType::ActorRef:
		case PropertyType::ComponentRef:
		case PropertyType::String:
			if (auto _v = _node.TryGet<String>(_key)) return PropertyValue(_v->StdString());
			break;
		case PropertyType::Vector2:
			if (auto _v = _node.TryGet<std::vector<Float>>(_key); _v && _v->size() == 2)
				return PropertyValue(FVector2(static_cast<float>((*_v)[0]), static_cast<float>((*_v)[1])));
			break;
		case PropertyType::Color:
			if (auto _v = _node.TryGet<std::vector<Int>>(_key); _v && _v->size() == 4)
				return PropertyValue(Color(
					static_cast<std::uint8_t>(static_cast<int>((*_v)[0])), static_cast<std::uint8_t>(static_cast<int>((*_v)[1])),
					static_cast<std::uint8_t>(static_cast<int>((*_v)[2])), static_cast<std::uint8_t>(static_cast<int>((*_v)[3]))));
			break;
		}
		return std::nullopt;
	}

	template <typename C>
	C* FindComponent(const std::vector<std::unique_ptr<C>>& _components, const std::string& _className)
	{
		for (const std::unique_ptr<C>& _component : _components)
			if (ClassNameOf(*_component) == _className)
				return _component.get();
		return nullptr;
	}

	std::vector<const PropertyInfo*> PropertiesOf(const IGameObject& _object)
	{
		const ClassInfo* _class = ClassRegistry::Find(ClassNameOf(_object));
		return _class ? _class->GetProperties() : std::vector<const PropertyInfo*>{ };
	}

	SaveNodePtr WriteProperties(IGameObject& _object)
	{
		auto _props = std::make_shared<SaveNode>("props");
		for (const PropertyInfo* _property : PropertiesOf(_object))
			WriteValue(*_props, String(_property->name), _property->get(&_object));
		return _props;
	}

	// The values first, while the actors are spawned; the references (KPROPERTY(Actor*, ...)) once they all exist, since one may
	// point to an actor further in the file.
	enum class Pass { Values, References };

	// While PasteActors reads: the ids of the copied actors (hex) -> the ids of their new copies.
	const std::unordered_map<std::string, std::string>* g_idRemap = nullptr;

	// "id" or "id/Component": an id of a copied actor becomes the one of its copy.
	void RemapReference(std::string& _reference)
	{
		const std::string _id = _reference.substr(0, _reference.find('/'));
		if (const auto _new = g_idRemap->find(_id); _new != g_idRemap->end())
			_reference = _new->second + _reference.substr(_id.size());
	}

	void ReadProperties(IGameObject& _object, const SaveNode& _node, const Pass _pass = Pass::Values)
	{
		const std::optional<SaveNode> _props = _node.GetNode("props");
		if (!_props) return;

		for (const PropertyInfo* _property : PropertiesOf(_object))
		{
			if (_property->IsReference() != (_pass == Pass::References)) continue;
			if (std::optional<PropertyValue> _value = ReadValue(*_props, _property->type, String(_property->name)))
			{
				if (_pass == Pass::References && g_idRemap) RemapReference(std::get<std::string>(*_value));
				_property->set(&_object, *_value);
				if (_pass == Pass::References && !std::get<std::string>(*_value).empty() && !_property->getObject(&_object))
					LOG_WARNING("Scene: " + _object.GetName().StdString() + "." + _property->name + " points to an object that is not in the scene (or not a " + _property->referenceClass + "), set to none");
			}
		}
	}

	// Second pass of an actor or a widget: the references of the object and of its components.
	template <typename T>
	void ReadReferences(T& _object, const SaveNode& _node)
	{
		ReadProperties(_object, _node, Pass::References);
		for (const SaveNodePtr& _componentNode : _node.GetNodes("components"))
			if (auto* _component = FindComponent(_object.GetComponents(), _componentNode->Get<String>("class").StdString()))
				ReadProperties(*_component, *_componentNode, Pass::References);
	}

	template <typename T>
	void ReadAllReferences(const std::vector<SaveNodePtr>& _nodes, const std::vector<T*>& _spawned)
	{
		for (size_t _i = 0; _i < _nodes.size(); ++_i)
			if (_spawned[_i]) ReadReferences(*_spawned[_i], *_nodes[_i]);
	}

	// ---- What an actor and a widget have in common: name, active, transform, props, parent, components.

	template <typename T>
	void WriteCommon(SaveNode& _node, T& _object, const int& _parent)
	{
		_node.Set("class", String(ClassNameOf(_object)));
		_node.Set("name", _object.GetName());
		_node.Set("active", Bool(_object.IsActive()));
		_node.Set("position", std::vector<Float>{ Float(_object.GetPosition().x), Float(_object.GetPosition().y) });
		_node.Set("rotation", _object.GetRotation().ToDegrees());
		_node.Set("scale", std::vector<Float>{ Float(_object.GetScale().x), Float(_object.GetScale().y) });
		_node.Set("props", WriteProperties(_object));
		_node.Set("parent", Int(_parent));
		if constexpr (std::is_base_of_v<Actor, T>)
			_node.Set("id", String(std::format("{:016x}", _object.GetId())));	// what references to it remember
	}

	template <typename T>
	void ReadCommon(const SaveNode& _node, T& _object)
	{
		_object.SetName(_node.Get<String>("name", _object.GetName()));
		if constexpr (std::is_base_of_v<Actor, T>)
		{
			// A scene saved before ids existed keeps the random one given at spawn.
			const std::string _id = _node.Get<String>("id", String("")).StdString();
			std::uint64_t _value = 0;
			if (!_id.empty() && std::from_chars(_id.data(), _id.data() + _id.size(), _value, 16).ec == std::errc() && _value != 0)
				_object.SetId(_value);
		}
		_object.SetActive(_node.Get<Bool>("active", Bool(true)));

		const std::vector<Float> _position = _node.Get<std::vector<Float>>("position");
		if (_position.size() == 2) _object.SetPosition(FVector2(static_cast<float>(_position[0]), static_cast<float>(_position[1])));
		_object.SetRotation(Angle::FromDegrees(_node.Get<Float>("rotation", Float(0.f))));
		const std::vector<Float> _scale = _node.Get<std::vector<Float>>("scale");
		if (_scale.size() == 2) _object.SetScale(FVector2(static_cast<float>(_scale[0]), static_cast<float>(_scale[1])));

		ReadProperties(_object, _node);
	}

	// A component is saved when the registry knows its class and it has something to say (a factory or properties).
	template <typename C>
	std::vector<SaveNodePtr> WriteComponents(const std::vector<std::unique_ptr<C>>& _components)
	{
		std::vector<SaveNodePtr> _nodes;
		for (const std::unique_ptr<C>& _component : _components)
		{
			const ClassInfo* _class = ClassRegistry::Find(ClassNameOf(*_component));
			if (!_class) continue;
			const bool _creatable = _class->componentFactory || _class->widgetComponentFactory;
			if (!_creatable && _class->GetProperties().empty()) continue;

			auto _node = std::make_shared<SaveNode>("component");
			_node->Set("class", String(ClassNameOf(*_component)));
			_node->Set("props", WriteProperties(*_component));
			_nodes.push_back(_node);
		}
		return _nodes;
	}

	void ReadComponents(Actor& _actor, const SaveNode& _node)
	{
		for (const SaveNodePtr& _componentNode : _node.GetNodes("components"))
		{
			const std::string _className = _componentNode->Get<String>("class").StdString();
			Component* _component = FindComponent(_actor.GetComponents(), _className);	// made by the actor's constructor
			if (!_component)
			{
				const ClassInfo* _class = ClassRegistry::Find(_className);
				if (_class && _class->componentFactory) _component = _class->componentFactory(&_actor);
				else LOG_WARNING("Scene: unknown component class '" + _className + "' on '" + _actor.GetName().StdString() + "', skipped");
			}
			if (_component) ReadProperties(*_component, *_componentNode);
		}
	}

	void ReadComponents(Widget& _widget, const SaveNode& _node)
	{
		for (const SaveNodePtr& _componentNode : _node.GetNodes("components"))
		{
			const std::string _className = _componentNode->Get<String>("class").StdString();
			WidgetComponent* _component = FindComponent(_widget.GetComponents(), _className);
			if (!_component)
			{
				const ClassInfo* _class = ClassRegistry::Find(_className);
				if (_class && _class->widgetComponentFactory) _component = _class->widgetComponentFactory(&_widget);
				else LOG_WARNING("Scene: unknown widget component class '" + _className + "' on '" + _widget.GetName().StdString() + "', skipped");
			}
			if (_component) ReadProperties(*_component, *_componentNode);
		}
	}

	template <typename T>
	int IndexOfParent(const std::vector<T*>& _saved, T* _child)
	{
		for (size_t _i = 0; _i < _saved.size(); ++_i)
		{
			const auto& _children = _saved[_i]->GetChildren();
			if (std::find(_children.begin(), _children.end(), _child) != _children.end())
				return static_cast<int>(_i);
		}
		return -1;
	}

	template <typename T>
	void LinkParents(const std::vector<SaveNodePtr>& _nodes, const std::vector<T*>& _spawned)
	{
		for (size_t _i = 0; _i < _nodes.size(); ++_i)
		{
			const int _parent = static_cast<int>(_nodes[_i]->Get<Int>("parent", Int(-1)));
			if (_spawned[_i] && _parent >= 0 && _parent < static_cast<int>(_spawned.size()) && _spawned[_parent])
				_spawned[_parent]->AddChild(_spawned[_i]);
		}
	}
}

bool Scene::Save(Level& _level, const std::filesystem::path& _path, const bool& _quiet)
{
	std::vector<Actor*> _savedActors;
	for (const std::unique_ptr<Actor>& _actor : _level.GetActorManager().GetActors())
	{
		const ClassInfo* _class = ClassRegistry::Find(ClassNameOf(*_actor));
		if (!_class || !_class->factory)
		{
			if (!_quiet) LOG_WARNING("Scene: '" + _actor->GetName().StdString() + "' (" + ClassNameOf(*_actor) + ") is not registered with KACTOR, not saved");
			continue;
		}
		_savedActors.push_back(_actor.get());
	}

	std::vector<SaveNodePtr> _actorNodes;
	for (Actor* _actor : _savedActors)
	{
		auto _node = std::make_shared<SaveNode>("actor");
		WriteCommon(*_node, *_actor, IndexOfParent(_savedActors, _actor));
		_node->Set("components", WriteComponents(_actor->GetComponents()));
		_actorNodes.push_back(_node);
	}

	std::vector<Widget*> _savedWidgets;
	for (const std::unique_ptr<Widget>& _widget : _level.GetHUD().GetWidgets())
	{
		const ClassInfo* _class = ClassRegistry::Find(ClassNameOf(*_widget));
		if (!_class || !_class->widgetFactory)
		{
			if (!_quiet) LOG_WARNING("Scene: widget '" + _widget->GetName().StdString() + "' (" + ClassNameOf(*_widget) + ") is not a registered widget class, not saved");
			continue;
		}
		_savedWidgets.push_back(_widget.get());
	}

	std::vector<SaveNodePtr> _widgetNodes;
	for (Widget* _widget : _savedWidgets)
	{
		auto _node = std::make_shared<SaveNode>("widget");
		WriteCommon(*_node, *_widget, IndexOfParent(_savedWidgets, _widget));
		_node->Set("components", WriteComponents(_widget->GetComponents()));
		_widgetNodes.push_back(_node);
	}

	SaveNode _root("Scene");
	_root.Set("version", Int(SCENE_VERSION));
	_root.Set("actors", std::move(_actorNodes));
	_root.Set("widgets", std::move(_widgetNodes));
	const Color& _background = _level.GetBackgroundColor();
	_root.Set("background", std::vector<Int>{ Int(_background.r), Int(_background.g), Int(_background.b), Int(_background.a) });

	std::error_code _error;
	if (_path.has_parent_path()) std::filesystem::create_directories(_path.parent_path(), _error);

	if (SaveManager::SaveToFile(_path, _root) != SaveResult::Success)
	{
		LOG_ERROR("Scene: cannot write " + _path.string());
		return false;
	}
	LOG(_quiet ? VerbosityType::VeryVerbose : VerbosityType::Display, "Scene saved: " + _path.string() + " (" + std::to_string(_savedActors.size()) + " actors, " + std::to_string(_savedWidgets.size()) + " widgets)");
	return true;
}

bool Scene::Load(Level& _level, const std::filesystem::path& _path)
{
	LOG(VerbosityType::Verbose, "Scene: reading " + _path.generic_string());
	SaveNode _root;
	const SaveResult _result = SaveManager::LoadFromFile(_path, _root);
	if (_result != SaveResult::Success)
	{
		LOG_ERROR("Scene: cannot read " + _path.string() + ": " + ToString(_result));
		return false;
	}

	// ---- Level settings (a scene saved before they existed keeps the level's own color)
	if (auto _background = _root.TryGet<std::vector<Int>>("background"); _background && _background->size() == 4)
		_level.SetBackgroundColor(Color(
			static_cast<std::uint8_t>(static_cast<int>((*_background)[0])), static_cast<std::uint8_t>(static_cast<int>((*_background)[1])),
			static_cast<std::uint8_t>(static_cast<int>((*_background)[2])), static_cast<std::uint8_t>(static_cast<int>((*_background)[3]))));

	// ---- Actors
	const std::vector<SaveNodePtr> _actorNodes = _root.GetNodes("actors");
	std::vector<Actor*> _actors(_actorNodes.size(), nullptr);

	for (size_t _i = 0; _i < _actorNodes.size(); ++_i)
	{
		const SaveNode& _node = *_actorNodes[_i];
		const std::string _className = _node.Get<String>("class").StdString();
		const ClassInfo* _class = ClassRegistry::Find(_className);
		if (!_class || !_class->factory)
		{
			LOG_WARNING("Scene: unknown actor class '" + _className + "' (script removed or not built yet), skipped");
			continue;
		}

		Actor* _actor = _class->factory(&_level);
		_actors[_i] = _actor;
		ReadComponents(*_actor, _node);
		ReadCommon(_node, *_actor);
		LOG(VerbosityType::Verbose, "Scene: actor " + _actor->GetName().StdString() + " (" + _className + ", " + std::to_string(_actor->GetComponents().size()) + " components)");
	}
	LinkParents(_actorNodes, _actors); // every actor exists now

	// ---- HUD widgets
	const std::vector<SaveNodePtr> _widgetNodes = _root.GetNodes("widgets");
	std::vector<Widget*> _widgets(_widgetNodes.size(), nullptr);

	for (size_t _i = 0; _i < _widgetNodes.size(); ++_i)
	{
		const SaveNode& _node = *_widgetNodes[_i];
		const std::string _className = _node.Get<String>("class").StdString();
		const ClassInfo* _class = ClassRegistry::Find(_className);
		if (!_class || !_class->widgetFactory)
		{
			LOG_WARNING("Scene: unknown widget class '" + _className + "', skipped");
			continue;
		}

		Widget* _widget = _class->widgetFactory(&_level);
		_widgets[_i] = _widget;
		ReadComponents(*_widget, _node);	// before the transform: components follow their owner
		ReadCommon(_node, *_widget);
		LOG(VerbosityType::Verbose, "Scene: widget " + _widget->GetName().StdString() + " (" + _className + ", " + std::to_string(_widget->GetComponents().size()) + " components)");
	}
	LinkParents(_widgetNodes, _widgets);

	// ---- References between objects: every actor and widget exists now
	ReadAllReferences(_actorNodes, _actors);
	ReadAllReferences(_widgetNodes, _widgets);

	LOG(_level.GetWorld()->GetLevelManager()->IsChangeSilent() ? VerbosityType::Log : VerbosityType::Display, "Scene: loaded " + _path.generic_string() + " (" + std::to_string(_actorNodes.size()) + " actors, " + std::to_string(_widgetNodes.size()) + " widgets)");

	return true;
}

std::filesystem::path Scene::Resolve(const std::string& _nameOrPath)
{
	std::filesystem::path _path = _nameOrPath;
	if (_path.extension() != EXTENSION) _path += EXTENSION;
	if (!_path.has_parent_path()) _path = std::filesystem::path("Content/Scenes") / _path;
	return _path;
}

void Scene::Open(Engine& _engine, const std::string& _nameOrPath)
{
	LOG(VerbosityType::Log, "Scene: opening " + Resolve(_nameOrPath).generic_string());
	_engine.GetLevelManager()->SetLevel<SceneLevel>()->SetScenePath(Resolve(_nameOrPath));
}

void SceneLevel::InitLevel()
{
	Level::InitLevel();

	if (!scenePath.empty())
		Scene::Load(*this, scenePath);
}

// ---- Copy / paste of actors (editor)

bool Scene::CopyActors(const std::vector<Actor*>& _actors, const std::filesystem::path& _path)
{
	// The actors and everything under them, each once, in hierarchy order (a parent before its children).
	std::vector<Actor*> _saved;
	const std::function<void(Actor*)> _add = [&](Actor* _actor)
	{
		if (!_actor || std::find(_saved.begin(), _saved.end(), _actor) != _saved.end()) return;
		const ClassInfo* _class = ClassRegistry::Find(ClassNameOf(*_actor));
		if (!_class || !_class->factory)
		{
			LOG_WARNING("Scene: '" + _actor->GetName().StdString() + "' (" + ClassNameOf(*_actor) + ") is not registered with KACTOR, not copied");
			return;
		}
		_saved.push_back(_actor);
		for (Actor* _child : _actor->GetChildren()) _add(_child);
	};
	for (Actor* _actor : _actors) _add(_actor);
	if (_saved.empty()) return false;

	std::vector<SaveNodePtr> _nodes;
	for (Actor* _actor : _saved)
	{
		auto _node = std::make_shared<SaveNode>("actor");
		WriteCommon(*_node, *_actor, IndexOfParent(_saved, _actor));
		_node->Set("components", WriteComponents(_actor->GetComponents()));
		_nodes.push_back(_node);
	}
	SaveNode _root("Actors");
	_root.Set("version", Int(SCENE_VERSION));
	_root.Set("actors", std::move(_nodes));

	std::error_code _error;
	if (_path.has_parent_path()) std::filesystem::create_directories(_path.parent_path(), _error);
	if (SaveManager::SaveToFile(_path, _root) != SaveResult::Success)
	{
		LOG_ERROR("Scene: cannot write " + _path.string());
		return false;
	}
	LOG(VerbosityType::Verbose, "Scene: copied " + std::to_string(_saved.size()) + " actor(s)");
	return true;
}

std::vector<Actor*> Scene::PasteActors(Level& _level, const std::filesystem::path& _path)
{
	SaveNode _root;
	if (SaveManager::LoadFromFile(_path, _root) != SaveResult::Success) return {};

	const std::vector<SaveNodePtr> _nodes = _root.GetNodes("actors");
	std::vector<Actor*> _actors(_nodes.size(), nullptr);
	std::unordered_map<std::string, std::string> _remap;
	for (size_t _i = 0; _i < _nodes.size(); ++_i)
	{
		const std::string _className = _nodes[_i]->Get<String>("class").StdString();
		const ClassInfo* _class = ClassRegistry::Find(_className);
		if (!_class || !_class->factory)
		{
			LOG_WARNING("Scene: unknown actor class '" + _className + "', not pasted");
			continue;
		}
		Actor* _actor = _class->factory(&_level);
		const std::uint64_t _newId = _actor->GetId();	// the random one it got at spawn
		ReadComponents(*_actor, *_nodes[_i]);
		ReadCommon(*_nodes[_i], *_actor);				// gives it the copied actor's id...
		_remap[std::format("{:016x}", _actor->GetId())] = std::format("{:016x}", _newId);
		_actor->SetId(_newId);							// ...two actors must not share one
		_actors[_i] = _actor;
	}
	LinkParents(_nodes, _actors);

	g_idRemap = &_remap;
	ReadAllReferences(_nodes, _actors);
	g_idRemap = nullptr;

	std::vector<Actor*> _roots;
	for (Actor* _actor : _actors)
		if (_actor && !_actor->GetParent()) _roots.push_back(_actor);
	LOG(VerbosityType::Verbose, "Scene: pasted " + std::to_string(_nodes.size()) + " actor(s)");
	return _roots;
}
