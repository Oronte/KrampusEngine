#pragma once
#include "GameFramework/Level.h"
#include "Reflection/EventList.h"

// Reflection for the editor and for scenes (.ka7). Usage, in the header of an Actor or Component:
//
//   class Spinner : public Actor
//   {
//       KACTOR(Spinner, Actor)                           // registers the class (KCLASS for a component)
//   public:
//       Spinner(Level* _level);
//       KPROPERTY(Float, speed) = 90.f;                  // shown in the Inspector, saved in scenes
//       KPROPERTY_RANGE(Float, health, 0.f, 100.f) = 100.f;
//       KPROPERTY_READONLY(Int, kills) = 0;              // shown but not editable
//       KPROPERTY(Actor*, target) = nullptr;             // another actor of the scene, picked in the Inspector
//       KPROPERTY(SpriteComponent*, sprite) = nullptr;   // a component of an actor of the scene
//   };
//
// Supported types: bool, int, float, std::string, FVector2, Color and the engine's Bool, Int, Float, String.
// References: a pointer to Actor, Component or any class derived from them (include its header). Only objects of that class can be
// picked; the scene saves which one it is and finds it again when it loads; the pointer becomes nullptr when its target is destroyed.
// Components that scenes and the editor can add by name use KCOMPONENT(Class, Parent) with a public constructor Class(Actor*).
// KPROPERTY leaves the following members public, like a Unity public field.

namespace Krampus
{

	enum class PropertyType : std::uint8_t
	{
		Bool,
		Int,
		Float,
		String,
		Vector2,
		Color,
		Flags,		// stored as an int: one bit per name in PropertyInfo::flagNames, shown as check boxes
		Enum,		// stored as an int: index in PropertyInfo::flagNames, shown as a drop-down list
		Texture,	// stored as a string ("Folder/Name.png", relative to Content/Textures), picked from a list of pictures
		Font,		// stored as a string ("Folder/Name.ttf", relative to Content/Fonts), picked from a list
		Sound,		// stored as a string ("Folder/Name.mp3", relative to Content/Audio, empty = none), picked from a list
		Scene,		// stored as a string (name of a scene of Content/Scenes, without .ka7), picked from a list
		Events,		// stored as a string (EventList::Encode), edited as a list of function calls
		InputBindings,	// stored as a string (InputComponent::EncodeBindings): input events and their function calls
		MessageBindings,	// the same, but the names are network messages typed by the user
		Shape,			// stored as a string (name of a shape of Content/Shapes, without .kshape), picked from a list, edited in the Shape Editor
		ActorRef,		// a pointer to an actor of the level, stored as a string (Reflection::EncodeReference): see PropertyInfo::getObject
		ComponentRef	// a pointer to a component of an actor of the level, stored the same way
	};

	/// <summary>
	/// One reflected member: its name, type, and type-erased access. Objects are passed as IGameObject*,
	/// the common base of Actor and Component, so the cast to the owning class is always correct.
	/// </summary>
	struct PropertyInfo
	{
		std::string													name;
		PropertyType												type		= PropertyType::Float;
		std::function<PropertyValue(IGameObject*)>					get;
		std::function<void(IGameObject*, const PropertyValue&)>		set;
		float														min			= 0.f;	// min == max: no limit
		float														max			= 0.f;
		bool														readOnly	= false;
		std::vector<std::string>									flagNames;	// for Flags: name of bit 0, bit 1... ; for Enum: name of value 0, 1...
		std::function<bool(IGameObject*)>							visible;	// empty: always shown; else the Inspector hides the property when it returns false (the value is still saved)

		// ActorRef and ComponentRef only: the pointer itself, the class it points to, and whether an object can be put in it.
		std::function<IGameObject*(IGameObject*)>					getObject;
		std::function<void(IGameObject*, IGameObject*)>				setObject;	// an object of another class is set as nullptr
		std::function<bool(IGameObject*)>							accepts;
		std::string													referenceClass;	// "Actor", "SpriteComponent"...

		inline bool IsReference() const { return type == PropertyType::ActorRef || type == PropertyType::ComponentRef; }
	};

	/// <summary>
	/// A function the editor can call (KFUNCTION, or described in EngineClasses.cpp): its name and, if it takes one, the type of its argument.
	/// </summary>
	struct FunctionInfo
	{
		std::string														name;
		bool															hasArgument		= false;
		PropertyType													argumentType	= PropertyType::Bool;
		std::function<void(IGameObject*, const PropertyValue*)>			call;
	};

	namespace Reflection
	{
		template <typename T>
		inline constexpr bool AlwaysFalse = false;

		/// <summary>The engine the event lists work for while one is being invoked (target "Game": open scene, quit).</summary>
		inline Engine*& CurrentEngine()
		{
			static Engine* _engine = nullptr;
			return _engine;
		}

		/// <summary>True for Actor*, Component* and pointers to their derived classes: a reference to another object of the level.</summary>
		template <typename T>
		inline constexpr bool IsReference = std::is_pointer_v<T>
			&& (std::is_base_of_v<Actor, std::remove_cv_t<std::remove_pointer_t<T>>> || std::is_base_of_v<Component, std::remove_cv_t<std::remove_pointer_t<T>>>);

		/// <summary>
		/// What a scene saves for a reference: "" (nullptr), "<actor id>" for an actor, "<actor id>/<ComponentClass>" for a component.
		/// </summary>
		std::string EncodeReference(IGameObject* _target);

		/// <summary>
		/// The object EncodeReference gave _text for, in the level of _context (the object that holds the reference). nullptr when it is
		/// not there (yet: a scene sets its references once all its actors are spawned).
		/// </summary>
		IGameObject* ResolveReference(IGameObject* _context, const std::string& _text);

		/// <summary>The name of a C++ type without namespace ("SpriteComponent").</summary>
		std::string TypeName(const std::type_info& _type);

		template <typename T>
		constexpr PropertyType TypeOf()
		{
			static_assert(!std::is_base_of_v<IGameObject, T>, "KPROPERTY: an actor or a component is reflected as a pointer: KPROPERTY(Actor*, name) = nullptr;");
			if constexpr (IsReference<T>)
				return std::is_base_of_v<Actor, std::remove_cv_t<std::remove_pointer_t<T>>> ? PropertyType::ActorRef : PropertyType::ComponentRef;
			else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, Bool>)			return PropertyType::Bool;
			else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, Int>)		return PropertyType::Int;
			else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, Float>)	return PropertyType::Float;
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, String>)	return PropertyType::String;
			else if constexpr (std::is_same_v<T, FVector2>)								return PropertyType::Vector2;
			else if constexpr (std::is_same_v<T, Color>)								return PropertyType::Color;
			else if constexpr (std::is_same_v<T, EventList>)							return PropertyType::Events;
			else static_assert(AlwaysFalse<T>, "KPROPERTY: unsupported type");
		}

		template <typename T>
		PropertyValue ToValue(const T& _value)
		{
			if constexpr (std::is_same_v<T, Bool>)			return static_cast<bool>(_value);
			else if constexpr (std::is_same_v<T, Int>)		return static_cast<int>(_value);
			else if constexpr (std::is_same_v<T, Float>)	return static_cast<float>(_value);
			else if constexpr (std::is_same_v<T, String>)	return _value.StdString();
			else if constexpr (std::is_same_v<T, EventList>)	return _value.Encode();
			else											return _value;
		}

		template <typename T>
		T FromValue(const PropertyValue& _value)
		{
			if constexpr (std::is_same_v<T, bool>)				return std::get<bool>(_value);
			else if constexpr (std::is_same_v<T, Bool>)			return Bool(std::get<bool>(_value));
			else if constexpr (std::is_same_v<T, int>)			return std::get<int>(_value);
			else if constexpr (std::is_same_v<T, Int>)			return Int(std::get<int>(_value));
			else if constexpr (std::is_same_v<T, float>)		return std::get<float>(_value);
			else if constexpr (std::is_same_v<T, Float>)		return Float(std::get<float>(_value));
			else if constexpr (std::is_same_v<T, std::string>)	return std::get<std::string>(_value);
			else if constexpr (std::is_same_v<T, String>)		return String(std::get<std::string>(_value));
			else if constexpr (std::is_same_v<T, FVector2>)		return std::get<FVector2>(_value);
			else if constexpr (std::is_same_v<T, EventList>)	return EventList::Decode(std::get<std::string>(_value));
			else												return std::get<Color>(_value);
		}
	}

	/// <summary>
	/// What the registry knows about a class: its parent, its reflected members and, for actors, how to spawn it.
	/// </summary>
	class ClassInfo
	{
		mutable bool											built		= false;
		mutable std::vector<PropertyInfo>						properties;
		mutable std::vector<FunctionInfo>						functions;

		void Build() const;
		std::vector<void(*)(ClassInfo&)>						declarers;	// one per KPROPERTY, run on first use

	public:
		std::string												name;
		std::string												parentName;
		std::function<Actor*(Level*)>							factory;					// set for actors only
		std::function<Component*(Actor*)>						componentFactory;			// set for actor components
		std::function<Widget*(Level*)>							widgetFactory;				// set for HUD widgets
		std::function<WidgetComponent*(Widget*)>				widgetComponentFactory;		// set for widget components

		/// <summary>
		/// Reflects a value that is not a plain member: read and written through functions (Get/Set methods of the engine classes).
		/// T must be bool, int, float, std::string, FVector2 or Color.
		/// </summary>
		template <typename Class, typename T>
		void AddAccessor(const char* _name, std::function<T(Class&)> _get, std::function<void(Class&, const T&)> _set,
			float _min = 0.f, float _max = 0.f, bool _readOnly = false)
		{
			if constexpr (Reflection::IsReference<T>)
			{
				using Target = std::remove_cv_t<std::remove_pointer_t<T>>;
				AddReference<Class, Target>(_name,
					[_get](Class& _object) -> Target* { return _get(_object); },
					[_set](Class& _object, Target* _target) { _set(_object, _target); }, _readOnly);
			}
			else
			{
				PropertyInfo _property;
				_property.name = _name;
				_property.type = Reflection::TypeOf<T>();
				_property.min = _min;
				_property.max = _max;
				_property.readOnly = _readOnly;
				_property.get = [_get](IGameObject* _object)
				{
					return PropertyValue(_get(*static_cast<Class*>(_object)));
				};
				_property.set = [_set](IGameObject* _object, const PropertyValue& _value)
				{
					_set(*static_cast<Class*>(_object), Reflection::FromValue<T>(_value));
				};
				properties.push_back(std::move(_property));
			}
		}

		/// <summary>
		/// Changes how the last added property is shown and saved: an int becomes Flags (check boxes named _flagNames),
		/// a string becomes a Texture (picked from the Content/Textures list).
		/// </summary>
		void SetLastPropertyType(const PropertyType& _type, std::vector<std::string> _flagNames = { })
		{
			properties.back().type = _type;
			properties.back().flagNames = std::move(_flagNames);
		}

		/// <summary>
		/// The last added property is only shown in the Inspector while _visible returns true (a radius only matters for a circle).
		/// </summary>
		template <typename Class>
		void SetLastPropertyVisible(std::function<bool(Class&)> _visible)
		{
			properties.back().visible = [_visible](IGameObject* _object)
			{
				return _visible(*static_cast<Class*>(_object));
			};
		}

		/// <summary>
		/// Reflects a pointer to another object of the level (Target: Actor, Component or a class derived from them).
		/// Its value is the text of Reflection::EncodeReference; getObject / setObject reach the pointer itself.
		/// </summary>
		template <typename Class, typename Target>
		void AddReference(const char* _name, std::function<Target*(Class&)> _get, std::function<void(Class&, Target*)> _set, bool _readOnly = false)
		{
			PropertyInfo _property;
			_property.name = _name;
			_property.type = std::is_base_of_v<Actor, Target> ? PropertyType::ActorRef : PropertyType::ComponentRef;
			_property.readOnly = _readOnly;
			_property.referenceClass = Reflection::TypeName(typeid(Target));
			_property.accepts = [](IGameObject* _object) { return dynamic_cast<Target*>(_object) != nullptr; };
			_property.getObject = [_get](IGameObject* _object) -> IGameObject*
			{
				return _get(*static_cast<Class*>(_object));
			};
			_property.setObject = [_set](IGameObject* _object, IGameObject* _target)
			{
				_set(*static_cast<Class*>(_object), dynamic_cast<Target*>(_target));
			};
			_property.get = [_get](IGameObject* _object)
			{
				return PropertyValue(Reflection::EncodeReference(_get(*static_cast<Class*>(_object))));
			};
			_property.set = [_set](IGameObject* _object, const PropertyValue& _value)
			{
				const std::string* _text = std::get_if<std::string>(&_value);
				_set(*static_cast<Class*>(_object), _text ? dynamic_cast<Target*>(Reflection::ResolveReference(_object, *_text)) : nullptr);
			};
			properties.push_back(std::move(_property));
		}

		template <typename Class, typename T>
		void AddProperty(const char* _name, T Class::* _member, float _min = 0.f, float _max = 0.f, bool _readOnly = false)
		{
			if constexpr (Reflection::IsReference<T>)
			{
				using Target = std::remove_cv_t<std::remove_pointer_t<T>>;
				AddReference<Class, Target>(_name,
					[_member](Class& _object) -> Target* { return _object.*_member; },
					[_member](Class& _object, Target* _target) { _object.*_member = _target; }, _readOnly);
			}
			else
			{
				PropertyInfo _property;
				_property.name = _name;
				_property.type = Reflection::TypeOf<T>();
				_property.min = _min;
				_property.max = _max;
				_property.readOnly = _readOnly;
				_property.get = [_member](IGameObject* _object)
				{
					return Reflection::ToValue<T>(static_cast<Class*>(_object)->*_member);
				};
				_property.set = [_member](IGameObject* _object, const PropertyValue& _value)
				{
					static_cast<Class*>(_object)->*_member = Reflection::FromValue<T>(_value);
				};
				properties.push_back(std::move(_property));
			}
		}

		void AddDeclarer(void(*_declare)(ClassInfo&))
		{
			declarers.push_back(_declare);
		}

		/// <summary>
		/// Reflected members of this class, parents' first.
		/// </summary>
		std::vector<const PropertyInfo*> GetProperties() const;

		/// <summary>
		/// Makes a function callable from the editor (event lists): no argument.
		/// </summary>
		template <typename Class>
		void AddFunction(const char* _name, std::function<void(Class&)> _call)
		{
			FunctionInfo _function;
			_function.name = _name;
			_function.call = [_call](IGameObject* _object, const PropertyValue*)
			{
				_call(*static_cast<Class*>(_object));
			};
			functions.push_back(std::move(_function));
		}

		/// <summary>
		/// Makes a function callable from the editor: one argument of type T (bool, int, float, std::string, FVector2 or Color).
		/// </summary>
		template <typename Class, typename T>
		void AddFunction(const char* _name, std::function<void(Class&, const T&)> _call)
		{
			static_assert(!Reflection::IsReference<T>, "KFUNCTION_ARG: an actor or a component cannot be an argument (give the function a KPROPERTY to read instead)");
			FunctionInfo _function;
			_function.name = _name;
			_function.hasArgument = true;
			_function.argumentType = Reflection::TypeOf<T>();
			_function.call = [_call](IGameObject* _object, const PropertyValue* _value)
			{
				if (_value) _call(*static_cast<Class*>(_object), Reflection::FromValue<T>(*_value));
			};
			functions.push_back(std::move(_function));
		}

		/// <summary>Changes how the argument of the last added function is edited (a string that is the name of a scene, for example).</summary>
		void SetLastFunctionArgumentType(const PropertyType& _type)
		{
			functions.back().argumentType = _type;
		}

		/// <summary>
		/// Functions of this class that event lists can call, parents' first.
		/// </summary>
		std::vector<const FunctionInfo*> GetFunctions() const;
	};

	class ClassRegistry
	{
	public:
		static ClassInfo& FindOrAdd(const std::string& _name);

		/// <returns>The class, or nullptr. Namespaces in the name are ignored.</returns>
		static ClassInfo* Find(const std::string& _name);

		/// <returns>Every class that can be spawned by name, sorted by name.</returns>
		static std::vector<ClassInfo*> GetActorClasses();

		/// <returns>Every actor component that can be added by name, sorted by name.</returns>
		static std::vector<ClassInfo*> GetComponentClasses();

		/// <returns>Every HUD widget that can be created by name, sorted by name.</returns>
		static std::vector<ClassInfo*> GetWidgetClasses();

		/// <returns>Every widget component that can be added by name, sorted by name.</returns>
		static std::vector<ClassInfo*> GetWidgetComponentClasses();
	};

	/// <returns>The name of the object's class without compiler decoration or namespace ("SpriteComponent").</returns>
	std::string ClassNameOf(const IGameObject& _object);

	/// <summary>
	/// Describes the classes of the engine itself (actors, components, widgets) in Reflection/EngineClasses.cpp.
	/// Called by the registry the first time it is used.
	/// </summary>
	void RegisterEngineClasses();

	// Static helpers used by the macros below.
	struct ClassRegistrar
	{
		ClassRegistrar(const char* _name, const char* _parentName);
	};

	struct PropertyRegistrar
	{
		PropertyRegistrar(const char* _className, void(*_declare)(ClassInfo&));
	};

	struct ActorRegistrar
	{
		ActorRegistrar(const char* _className, Actor*(*_spawn)(Level*));
	};

	struct ComponentRegistrar
	{
		ComponentRegistrar(const char* _className, Component*(*_add)(Actor*));
	};

	struct WidgetRegistrar
	{
		WidgetRegistrar(const char* _className, Widget*(*_spawn)(Level*));
	};

	struct WidgetComponentRegistrar
	{
		WidgetComponentRegistrar(const char* _className, WidgetComponent*(*_add)(Widget*));
	};
}

// The checks run in a member function: the class is complete there, so that is_base_of can see its bases.
#define KCLASS(Class, Parent) \
public: \
	using ThisClass = Class; \
	static constexpr const char* KClassName = #Class; \
	const char* ClassName() const override { return KClassName; } \
private: \
	static inline ::Krampus::ClassRegistrar _KClassRegistrar{ #Class, #Parent }; \
	static void _KCheckParent() \
	{ \
		static_assert(!std::is_same_v<Class, Parent>, "KCLASS/KACTOR/KCOMPONENT: a class cannot be its own parent (give it another name)"); \
		static_assert(std::is_base_of_v<Parent, Class>, "KCLASS/KACTOR/KCOMPONENT: the parent given must be a base class of this class"); \
	} \
public:

// For actors: also lets the editor and scenes create the class by name. Needs a public constructor Class(Level*).
#define KACTOR(Class, Parent) \
	KCLASS(Class, Parent) \
private: \
	static ::Krampus::Actor* _KSpawn(::Krampus::Level* _level) { return _level->SpawnActor<Class>(); } \
	static inline ::Krampus::ActorRegistrar _KActorRegistrar{ #Class, &_KSpawn }; \
public:

// For components: also lets scenes and the editor ("Add Component") create the class by name. Needs a public constructor Class(Actor*).
#define KCOMPONENT(Class, Parent) \
	KCLASS(Class, Parent) \
private: \
	static ::Krampus::Component* _KAddComponent(::Krampus::Actor* _owner) { return _owner->AddComponent<Class>(); } \
	static inline ::Krampus::ComponentRegistrar _KComponentRegistrar{ #Class, &_KAddComponent }; \
public:

// For HUD widgets: also lets scenes and the editor ("Add Widget") create the class by name. Needs a public constructor Class(Level*).
#define KWIDGET(Class, Parent) \
	KCLASS(Class, Parent) \
private: \
	static ::Krampus::Widget* _KSpawnWidget(::Krampus::Level* _level) { return _level->SpawnWidget<Class>(); } \
	static inline ::Krampus::WidgetRegistrar _KWidgetRegistrar{ #Class, &_KSpawnWidget }; \
public:

// For widget components: also lets scenes and the editor add the class by name. Needs a public constructor Class(Widget*).
#define KWIDGETCOMPONENT(Class, Parent) \
	KCLASS(Class, Parent) \
private: \
	static ::Krampus::WidgetComponent* _KAddWidgetComponent(::Krampus::Widget* _owner) { return _owner->AddComponent<Class>(); } \
	static inline ::Krampus::WidgetComponentRegistrar _KWidgetComponentRegistrar{ #Class, &_KAddWidgetComponent }; \
public:

#define KPROPERTY_IMPL(Type, Name, Min, Max, ReadOnly) \
private: \
	static void _KDeclare_##Name(::Krampus::ClassInfo& _info) { _info.AddProperty<ThisClass, Type>(#Name, &ThisClass::Name, Min, Max, ReadOnly); } \
	static inline ::Krampus::PropertyRegistrar _KPropertyRegistrar_##Name{ KClassName, &_KDeclare_##Name }; \
public: \
	Type Name

#define KPROPERTY(Type, Name)						KPROPERTY_IMPL(Type, Name, 0.f, 0.f, false)
#define KPROPERTY_RANGE(Type, Name, Min, Max)		KPROPERTY_IMPL(Type, Name, Min, Max, false)
#define KPROPERTY_READONLY(Type, Name)				KPROPERTY_IMPL(Type, Name, 0.f, 0.f, true)

// Makes a method callable from an event list in the editor (like a Unity event). Put it just before the method:
//   KFUNCTION(Explode) void Explode();                      // no argument
//   KFUNCTION_ARG(Float, TakeDamage) void TakeDamage(const Float& _amount);   // one argument: bool, int, float, std::string, FVector2, Color (or the engine types)
#define KFUNCTION(Name) \
private: \
	static void _KDeclareFunction_##Name(::Krampus::ClassInfo& _info) { _info.AddFunction<ThisClass>(#Name, [](ThisClass& _object) { _object.Name(); }); } \
	static inline ::Krampus::PropertyRegistrar _KFunctionRegistrar_##Name{ KClassName, &_KDeclareFunction_##Name }; \
public:

#define KFUNCTION_ARG(Type, Name) \
private: \
	static void _KDeclareFunction_##Name(::Krampus::ClassInfo& _info) { _info.AddFunction<ThisClass, Type>(#Name, [](ThisClass& _object, const Type& _value) { _object.Name(_value); }); } \
	static inline ::Krampus::PropertyRegistrar _KFunctionRegistrar_##Name{ KClassName, &_KDeclareFunction_##Name }; \
public:
