#pragma once
#include "GameFramework/Level.h"

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
//   };
//
// Supported types: bool, int, float, std::string, FVector2, Color and the engine's Bool, Int, Float, String.
// Components that scenes and the editor can add by name use KCOMPONENT(Class, Parent) with a public constructor Class(Actor*).
// KPROPERTY leaves the following members public, like a Unity public field.

namespace Krampus
{

	using PropertyValue = std::variant<bool, int, float, std::string, FVector2, Color>;

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
		Scene		// stored as a string (name of a scene of Content/Scenes, without .ka7), picked from a list
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
	};

	namespace Reflection
	{
		template <typename T>
		inline constexpr bool AlwaysFalse = false;

		template <typename T>
		constexpr PropertyType TypeOf()
		{
			if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, Bool>)			return PropertyType::Bool;
			else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, Int>)		return PropertyType::Int;
			else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, Float>)	return PropertyType::Float;
			else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, String>)	return PropertyType::String;
			else if constexpr (std::is_same_v<T, FVector2>)								return PropertyType::Vector2;
			else if constexpr (std::is_same_v<T, Color>)								return PropertyType::Color;
			else static_assert(AlwaysFalse<T>, "KPROPERTY: unsupported type");
		}

		template <typename T>
		PropertyValue ToValue(const T& _value)
		{
			if constexpr (std::is_same_v<T, Bool>)			return static_cast<bool>(_value);
			else if constexpr (std::is_same_v<T, Int>)		return static_cast<int>(_value);
			else if constexpr (std::is_same_v<T, Float>)	return static_cast<float>(_value);
			else if constexpr (std::is_same_v<T, String>)	return _value.StdString();
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

		/// <summary>
		/// Changes how the last added property is shown and saved: an int becomes Flags (check boxes named _flagNames),
		/// a string becomes a Texture (picked from the Content/Textures list).
		/// </summary>
		void SetLastPropertyType(const PropertyType& _type, std::vector<std::string> _flagNames = { })
		{
			properties.back().type = _type;
			properties.back().flagNames = std::move(_flagNames);
		}

		template <typename Class, typename T>
		void AddProperty(const char* _name, T Class::* _member, float _min = 0.f, float _max = 0.f, bool _readOnly = false)
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

		void AddDeclarer(void(*_declare)(ClassInfo&))
		{
			declarers.push_back(_declare);
		}

		/// <summary>
		/// Reflected members of this class, parents' first.
		/// </summary>
		std::vector<const PropertyInfo*> GetProperties() const;
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
}

#define KCLASS(Class, Parent) \
public: \
	using ThisClass = Class; \
	static constexpr const char* KClassName = #Class; \
	const char* ClassName() const override { return KClassName; } \
private: \
	static inline ::Krampus::ClassRegistrar _KClassRegistrar{ #Class, #Parent }; \
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

#define KPROPERTY_IMPL(Type, Name, Min, Max, ReadOnly) \
private: \
	static void _KDeclare_##Name(::Krampus::ClassInfo& _info) { _info.AddProperty<ThisClass, Type>(#Name, &ThisClass::Name, Min, Max, ReadOnly); } \
	static inline ::Krampus::PropertyRegistrar _KPropertyRegistrar_##Name{ KClassName, &_KDeclare_##Name }; \
public: \
	Type Name

#define KPROPERTY(Type, Name)						KPROPERTY_IMPL(Type, Name, 0.f, 0.f, false)
#define KPROPERTY_RANGE(Type, Name, Min, Max)		KPROPERTY_IMPL(Type, Name, Min, Max, false)
#define KPROPERTY_READONLY(Type, Name)				KPROPERTY_IMPL(Type, Name, 0.f, 0.f, true)
