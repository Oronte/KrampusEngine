#include "Reflection.h"
#include "Actors/CameraActor.h"
#include "Actors/EmptyActor.h"
#include "Components/AnimationComponent.h"
#include "Components/AnimationComponentSM.h"
#include "Components/InputComponent.h"
#include "Components/AudioListenerComponent.h"
#include "Components/CameraComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/LabelComponent.h"
#include "Components/NetworkEventsComponent.h"
#include "Components/NetworkSyncComponent.h"
#include "Network/NetworkManager.h"
#include "Components/PhysicsComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/SpatialSoundComponent.h"
#include "Components/SpriteComponent.h"
#include "Core/Settings.h"
#include "Scene/Scene.h"
#include "UI/Widgets/ButtonWidget.h"
#include "UI/Widgets/WidgetComponents/AnimationWidgetComponent.h"
#include "UI/Widgets/WidgetComponents/ButtonWidgetComponent.h"
#include "UI/Widgets/WidgetComponents/ImageWidgetComponent.h"
#include "UI/Widgets/WidgetComponents/LabelWidgetComponent.h"

// Everything the Inspector shows for the classes of the engine. The classes themselves stay untouched: values are read
// and written through their public members and Get/Set methods. Classes of the game use KACTOR / KCOMPONENT / KPROPERTY.

using namespace Krampus;

namespace
{
	std::string TextureToString(ShapeObject& _shape);
	std::string SplitTexture(const std::string& _value, TextureExtensionType& _type);

	ClassInfo& Describe(const char* _name, const char* _parent)
	{
		ClassInfo& _info = ClassRegistry::FindOrAdd(_name);
		_info.parentName = _parent;
		return _info;
	}

	// Draw order and debug outline, for the components that are drawn (IDrawable) and/or can outline themselves (IDebuggable).
	template <typename C>
	void AddDebugProperty(ClassInfo& _info)
	{
		_info.AddAccessor<C, bool>("useDebug",
			[](C& _c) { return static_cast<bool>(_c.IsDebugging()); },
			[](C& _c, const bool& _v) { if (_v) _c.UseDebug(); else _c.DisableDebug(); });
	}

	template <typename C>
	void AddDrawableProperties(ClassInfo& _info)
	{
		_info.AddAccessor<C, int>("zOrder",		// 0 behind, 255 in front
			[](C& _c) { return static_cast<int>(_c.GetZOrder()); },
			[](C& _c, const int& _v) { _c.SetZOrder(static_cast<uint8_t>(std::clamp(_v, 0, 255))); }, 0.f, 255.f);
		AddDebugProperty<C>(_info);
	}

	// Sprite and ImageWidgetComponent have the same shape API.
	template <typename C>
	void AddShapeProperties(ClassInfo& _info)
	{
		// First: size and radius only make sense once the shape type is right when a scene is loaded.
		_info.AddAccessor<C, int>("shape",		// 0 rectangle, 1 circle
			[](C& _c) { return _c.GetShapeType() == ShapeType::Circle ? 1 : 0; },
			[](C& _c, const int& _v) { _c.GetShapeObject()->SetShapeType(_v == 1 ? ShapeType::Circle : ShapeType::Rectangle); }, 0.f, 1.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Rectangle", "Circle" });
		_info.AddAccessor<C, FVector2>("size",
			[](C& _c) { return _c.GetShapeType() == ShapeType::Rectangle ? _c.GetSize() : FVector2(); },
			[](C& _c, const FVector2& _v) { if (_c.GetShapeType() == ShapeType::Rectangle) _c.SetSize(_v); });
		_info.SetLastPropertyVisible<C>([](C& _c) { return _c.GetShapeType() == ShapeType::Rectangle; });	// size
		_info.AddAccessor<C, float>("radius",
			[](C& _c) { return _c.GetShapeType() == ShapeType::Circle ? static_cast<float>(_c.GetRadius()) : 0.f; },
			[](C& _c, const float& _v) { if (_c.GetShapeType() == ShapeType::Circle) _c.SetRadius(_v); }, 0.f, 0.f);
		_info.SetLastPropertyVisible<C>([](C& _c) { return _c.GetShapeType() == ShapeType::Circle; });	// radius
		_info.AddAccessor<C, int>("pointCount",
			[](C& _c) { return _c.GetShapeType() == ShapeType::Circle ? static_cast<int>(_c.GetPointCount()) : 0; },
			[](C& _c, const int& _v) { if (_c.GetShapeType() == ShapeType::Circle) _c.SetPointCount(static_cast<size_t>(std::max(3, _v))); });
		_info.SetLastPropertyVisible<C>([](C& _c) { return _c.GetShapeType() == ShapeType::Circle; });	// pointCount
		_info.AddAccessor<C, std::string>("texture",
			[](C& _c) { return TextureToString(*_c.GetShapeObject()); },
			[](C& _c, const std::string& _v)
			{
				TextureExtensionType _type = TextureExtensionType::PNG;
				const std::string _path = SplitTexture(_v, _type);
				// Another picture keeps how the old one was filtered, and shows all of itself (the old rectangle would not fit it).
				const Bool _smooth = _c.GetShapeObject()->GetTextureRef().IsSmooth();
				const Bool _repeated = _c.GetShapeObject()->GetTextureRef().IsRepeated();
				_c.SetTexture(_path, _type, IRect(), _repeated, _smooth);
				_c.GetShapeObject()->GetShape()->SetTexture(&_c.GetShapeObject()->GetTextureRef(), true);
			});
		_info.SetLastPropertyType(PropertyType::Texture);

		// Which part of the picture the shape shows (position and size in pixels of the picture), and how the picture is filtered.
		// Smooth and repeated belong to the picture itself: every shape that uses the same file shares them.
		_info.AddAccessor<C, FVector2>("textureRectPosition",
			[](C& _c) { return FVector2(static_cast<float>(_c.GetShapeObject()->GetShape()->GetTextureRect().GetPosition().x), static_cast<float>(_c.GetShapeObject()->GetShape()->GetTextureRect().GetPosition().y)); },
			[](C& _c, const FVector2& _v)
			{
				IRect _rect = _c.GetShapeObject()->GetShape()->GetTextureRect();
				_rect.SetPosition(IVector2(static_cast<int>(_v.x), static_cast<int>(_v.y)));
				_c.GetShapeObject()->GetShape()->SetTextureRect(_rect);
			});
		_info.AddAccessor<C, FVector2>("textureRectSize",
			[](C& _c) { return FVector2(static_cast<float>(_c.GetShapeObject()->GetShape()->GetTextureRect().GetSize().x), static_cast<float>(_c.GetShapeObject()->GetShape()->GetTextureRect().GetSize().y)); },
			[](C& _c, const FVector2& _v)
			{
				IRect _rect = _c.GetShapeObject()->GetShape()->GetTextureRect();
				_rect.SetSize(IVector2(std::max(1, static_cast<int>(_v.x)), std::max(1, static_cast<int>(_v.y))));
				_c.GetShapeObject()->GetShape()->SetTextureRect(_rect);
			});
		_info.AddAccessor<C, bool>("isSmooth",
			[](C& _c) { return static_cast<bool>(_c.GetShapeObject()->GetTextureRef().IsSmooth()); },
			[](C& _c, const bool& _v) { _c.GetShapeObject()->GetTextureRef().SetSmooth(Bool(_v)); });
		_info.AddAccessor<C, bool>("isRepeated",
			[](C& _c) { return static_cast<bool>(_c.GetShapeObject()->GetTextureRef().IsRepeated()); },
			[](C& _c, const bool& _v) { _c.GetShapeObject()->GetTextureRef().SetRepeated(Bool(_v)); });
		AddDrawableProperties<C>(_info);
	}

	// The three animation components hold a list of animations (clips of a sprite sheet) and the one they start with.
	// The list is a single string property (hidden in the Inspector, edited in the Animation Editor), see AnimationClips.
	template <typename C>
	void AddAnimationProperties(ClassInfo& _info)
	{
		_info.AddAccessor<C, std::string>("animations",
			[](C& _c) { return AnimationClips::Encode(_c.clips); },
			[](C& _c, const std::string& _v) { _c.clips = AnimationClips::Decode(_v); _c.ApplyClips(); });
		_info.SetLastPropertyVisible<C>([](C&) { return false; });
		_info.AddAccessor<C, std::string>("startAnimation",		// drawn as a list of the clips by the Inspector
			[](C& _c) { return _c.startAnimation.StdString(); },
			[](C& _c, const std::string& _v) { _c.startAnimation = String(_v); _c.ApplyClips(); });
		_info.SetLastPropertyVisible<C>([](C&) { return false; });
		_info.AddProperty<C, Bool>("playOnBeginPlay", &C::playOnBeginPlay);
	}

	// Start / Stop / Pause / Resume of the three animation components.
	template <typename C>
	void AddAnimationFunctions(ClassInfo& _info)
	{
		_info.AddFunction<C>("Start", [](C& _c) { _c.StartAnimation(); });
		_info.AddFunction<C>("Stop", [](C& _c) { _c.StopAnimation(); });
		_info.AddFunction<C>("Pause", [](C& _c) { _c.PauseAnimation(); });
		_info.AddFunction<C>("Resume", [](C& _c) { _c.ResumeAnimation(); });
	}

	// LabelComponent and LabelWidgetComponent have the same text API.
	template <typename C>
	void AddLabelProperties(ClassInfo& _info)
	{
		_info.AddAccessor<C, std::string>("font",
			[](C& _c) { return _c.GetFontFileName().StdString() + (_c.GetFontType() == FontExtensionType::TTF ? ".ttf" : ".otf"); },
			[](C& _c, const std::string& _v)
			{
				const bool _ttf = _v.size() > 4 && (_v.compare(_v.size() - 4, 4, ".ttf") == 0 || _v.compare(_v.size() - 4, 4, ".TTF") == 0);
				const bool _hasExtension = _v.size() > 4 && _v[_v.size() - 4] == '.';
				_c.SetFont(String(_hasExtension ? _v.substr(0, _v.size() - 4) : _v), _ttf ? FontExtensionType::TTF : FontExtensionType::OTF);
			});
		_info.SetLastPropertyType(PropertyType::Font);
		_info.AddAccessor<C, std::string>("text",
			[](C& _c) { return _c.GetString().StdString(); },
			[](C& _c, const std::string& _v) { _c.SetString(String(_v)); });
		_info.AddAccessor<C, int>("characterSize",
			[](C& _c) { return static_cast<int>(_c.GetCharacterSize()); },
			[](C& _c, const int& _v) { _c.SetCharacterSize(static_cast<unsigned int>(std::max(1, _v))); });
		_info.AddAccessor<C, Color>("fillColor",
			[](C& _c) { return Color(_c.GetFillColor()); },
			[](C& _c, const Color& _v) { _c.SetFillColor(_v); });
		_info.AddAccessor<C, Color>("outlineColor",
			[](C& _c) { return Color(_c.GetOutlineColor()); },
			[](C& _c, const Color& _v) { _c.SetOutlineColor(_v); });
		_info.AddAccessor<C, float>("outlineThickness",
			[](C& _c) { return static_cast<float>(_c.GetOutlineThickness()); },
			[](C& _c, const float& _v) { _c.SetOutlineThickness(_v); });
		_info.AddAccessor<C, float>("letterSpacing",
			[](C& _c) { return static_cast<float>(_c.GetLetterSpacing()); },
			[](C& _c, const float& _v) { _c.SetLetterSpacing(_v); });
		_info.AddAccessor<C, float>("lineSpacing",
			[](C& _c) { return static_cast<float>(_c.GetLineSpacing()); },
			[](C& _c, const float& _v) { _c.SetLineSpacing(_v); });
		_info.AddAccessor<C, int>("style",		// Bold, Italic, Underlined, StrikeThrough (SFML's bits 0 to 3)
			[](C& _c) { return static_cast<int>(static_cast<unsigned int>(_c.GetStyle())); },
			[](C& _c, const int& _v) { _c.SetStyle(static_cast<unsigned int>(_v & 15)); });
		_info.SetLastPropertyType(PropertyType::Flags, { "Bold", "Italic", "Underlined", "StrikeThrough" });
		_info.AddAccessor<C, FVector2>("origin",	// the point of the text that sits on the owner's position
			[](C& _c) { return _c.GetOrigin(); },
			[](C& _c, const FVector2& _v) { _c.SetOrigin(_v); });
		AddDrawableProperties<C>(_info);
	}

	// A setting that is stored in the section and applied right away to the running engine.
	template <typename C, typename T>
	void AddLive(ClassInfo& _info, const char* _name, T C::* _member, const float& _min = 0.f, const float& _max = 0.f)
	{
		_info.AddAccessor<C, T>(_name,
			[_member](C& _c) { return _c.*_member; },
			[_member](C& _c, const T& _v) { _c.*_member = _v; _c.Apply(); }, _min, _max);
	}

	// A Logger flag: the value lives in a static field of Logger.
	void AddLoggerFlag(ClassInfo& _info, const char* _name, Bool& _field)
	{
		Bool* _pointer = &_field;
		_info.AddAccessor<LoggerSettings, bool>(_name,
			[_pointer](LoggerSettings&) { return static_cast<bool>(*_pointer); },
			[_pointer](LoggerSettings&, const bool& _v) { *_pointer = Bool(_v); });
	}

	const char* ExtensionOf(const TextureExtensionType& _type)
	{
		static const char* _names[] = { "png", "jpg", "gif", "bmp", "tga", "psd", "hdr", "pic", "pnm" };
		return _names[static_cast<int>(_type)];
	}

	// "Folder/Name.png"
	std::string TextureToString(ShapeObject& _shape)
	{
		return _shape.GetTexturePath().StdString() + "." + ExtensionOf(_shape.GetTextureType());
	}

	// "Folder/Name.jpg" -> "Folder/Name" and JPG. No extension: PNG.
	std::string SplitTexture(const std::string& _value, TextureExtensionType& _type)
	{
		_type = TextureExtensionType::PNG;
		const size_t _dot = _value.rfind('.');
		const size_t _slash = _value.find_last_of("/\\");
		if (_dot == std::string::npos || (_slash != std::string::npos && _dot < _slash)) return _value;

		std::string _extension = _value.substr(_dot + 1);
		std::transform(_extension.begin(), _extension.end(), _extension.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
		for (int _i = 0; _i <= static_cast<int>(TextureExtensionType::PNM); ++_i)
			if (_extension == ExtensionOf(static_cast<TextureExtensionType>(_i)))
				_type = static_cast<TextureExtensionType>(_i);
		return _value.substr(0, _dot);
	}

	// Bit 0, bit 1... of CollisionChannel.
	const std::vector<std::string>& ChannelNames()
	{
		static const std::vector<std::string> _names =
		{
			"Wall", "Player", "Enemy", "Projectile", "Trigger", "StaticWorld", "DynamicWorld", "Destructible", "Terrain", "Water", "Foliage",
			"NPC", "Boss", "Vehicle", "Companion", "PlayerProjectile", "EnemyProjectile", "AreaOfEffect", "PhysicsProp", "Ragdoll", "Pickup",
			"Shield", "Interactable", "Vision", "AudioTrigger", "Checkpoint", "KillZone", "Camera", "Cursor", "NavMesh", "HitboxHead", "HitboxLimb"
		};
		return _names;
	}

	int ChannelToInt(const CollisionChannel& _channel)
	{
		return static_cast<int>(static_cast<std::uint32_t>(static_cast<std::uint64_t>(_channel)));
	}

	CollisionChannel IntToChannel(const int& _value)
	{
		return static_cast<CollisionChannel>(static_cast<std::uint64_t>(static_cast<std::uint32_t>(_value)));
	}
}

void Krampus::RegisterEngineClasses()
{
	// ================= Bases =================
	Describe("IGameObject", "");
	Describe("Actor", "IGameObject");
	Describe("Component", "IGameObject");
	Describe("Widget", "IGameObject").widgetFactory = [](Level* _level) -> Widget*
	{
		return _level->GetHUD().CreateWidget<Widget>(_level);
	};
	Describe("WidgetComponent", "IGameObject");

	// ================= Actors =================
	// CameraActor and EmptyActor describe themselves with KACTOR.

	// ================= Actor components =================
	{
		ClassInfo& _info = Describe("SpriteComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<SpriteComponent>(RectangleShapeData()); };
		AddShapeProperties<SpriteComponent>(_info);
	}

	{
		ClassInfo& _info = Describe("CollisionComponent", "Component");
		_info.AddProperty<CollisionComponent, EventList>("onCollisionEnter", &CollisionComponent::onEnterCalls);
		_info.AddProperty<CollisionComponent, EventList>("onCollisionExit", &CollisionComponent::onExitCalls);
		_info.componentFactory = [](Actor* _owner) -> Component*
		{
			// Without a sprite to copy the shape from, start with a box.
			if (_owner->GetComponent<SpriteComponent>())
				return _owner->AddComponent<CollisionComponent>();
			return _owner->AddComponent<CollisionComponent>(FVector2(100.f, 100.f));
		};
		_info.AddAccessor<CollisionComponent, int>("shape",	// 0 rectangle, 1 circle
			[](CollisionComponent& _c) { return _c.GetShapeType() == ShapeType::Circle ? 1 : 0; },
			[](CollisionComponent& _c, const int& _v) { _c.SetShapeType(_v == 1 ? ShapeType::Circle : ShapeType::Rectangle); }, 0.f, 1.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Rectangle", "Circle" });
		_info.AddAccessor<CollisionComponent, FVector2>("size",
			[](CollisionComponent& _c) { return _c.GetShapeType() == ShapeType::Rectangle ? _c.GetSize() : FVector2(); },
			[](CollisionComponent& _c, const FVector2& _v) { if (_c.GetShapeType() == ShapeType::Rectangle) _c.SetSize(_v); });
		_info.SetLastPropertyVisible<CollisionComponent>([](CollisionComponent& _c) { return _c.GetShapeType() == ShapeType::Rectangle; });	// size
		_info.AddAccessor<CollisionComponent, float>("radius",
			[](CollisionComponent& _c) { return _c.GetShapeType() == ShapeType::Circle ? static_cast<float>(_c.GetRadius()) : 0.f; },
			[](CollisionComponent& _c, const float& _v) { if (_c.GetShapeType() == ShapeType::Circle) _c.SetRadius(_v); });
		_info.SetLastPropertyVisible<CollisionComponent>([](CollisionComponent& _c) { return _c.GetShapeType() == ShapeType::Circle; });	// radius
		_info.AddAccessor<CollisionComponent, int>("channel",
			[](CollisionComponent& _c) { return ChannelToInt(_c.channel); },
			[](CollisionComponent& _c, const int& _v) { _c.channel = IntToChannel(_v); });
		_info.SetLastPropertyType(PropertyType::Flags, ChannelNames());
		_info.AddAccessor<CollisionComponent, int>("mask",
			[](CollisionComponent& _c) { return ChannelToInt(_c.mask); },
			[](CollisionComponent& _c, const int& _v) { _c.mask = IntToChannel(_v); });
		_info.SetLastPropertyType(PropertyType::Flags, ChannelNames());
		AddDebugProperty<CollisionComponent>(_info);	// the red outline of the collision shape
	}

	{
		ClassInfo& _info = Describe("PhysicsComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component*
		{
			// The box2d shape is built from the sprite when there is one.
			if (SpriteComponent* _sprite = _owner->GetComponent<SpriteComponent>())
			{
				if (_sprite->GetShapeType() == ShapeType::Circle)
					return _owner->AddComponent<PhysicsComponent>(Float(_sprite->GetRadius()));
				return _owner->AddComponent<PhysicsComponent>(FVector2(_sprite->GetSize()));
			}
			return _owner->AddComponent<PhysicsComponent>(FVector2(100.f, 100.f));
		};
		_info.AddAccessor<PhysicsComponent, int>("bodyType",	// 0 static, 1 kinematic, 2 dynamic
			[](PhysicsComponent& _c) { return _c.GetBodyType(); },
			[](PhysicsComponent& _c, const int& _v) { _c.SetBodyType(std::clamp(_v, 0, 2)); }, 0.f, 2.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Static", "Kinematic", "Dynamic" });
		_info.AddAccessor<PhysicsComponent, float>("linearDamping",
			[](PhysicsComponent& _c) { return static_cast<float>(_c.GetLinearDamping()); },
			[](PhysicsComponent& _c, const float& _v) { _c.SetLinearDamping(_v); });
		_info.AddAccessor<PhysicsComponent, float>("angularDamping",
			[](PhysicsComponent& _c) { return static_cast<float>(_c.GetAngularDamping()); },
			[](PhysicsComponent& _c, const float& _v) { _c.SetAngularDamping(_v); });
		_info.AddAccessor<PhysicsComponent, float>("gravityScale",
			[](PhysicsComponent& _c) { return static_cast<float>(_c.GetGravityScale()); },
			[](PhysicsComponent& _c, const float& _v) { _c.SetGravityScale(_v); });
		_info.AddAccessor<PhysicsComponent, FVector2>("linearVelocity",
			[](PhysicsComponent& _c) { return _c.GetLinearVelocity(); },
			[](PhysicsComponent& _c, const FVector2& _v) { _c.SetLinearVelocity(_v); });
		_info.AddAccessor<PhysicsComponent, float>("angularVelocity",
			[](PhysicsComponent& _c) { return static_cast<float>(_c.GetAngularVelocity()); },
			[](PhysicsComponent& _c, const float& _v) { _c.SetAngularVelocity(_v); });

		// Mass data: each value is changed alone, the two others are kept (SetMassData takes the three together).
		_info.AddAccessor<PhysicsComponent, float>("mass",
			[](PhysicsComponent& _c) { Float _m, _i; FVector2 _center; _c.GetMassData(_m, _i, _center); return static_cast<float>(_m); },
			[](PhysicsComponent& _c, const float& _v) { Float _m, _i; FVector2 _center; _c.GetMassData(_m, _i, _center); _c.SetMassData(Float(std::max(0.f, _v)), _i, _center); });
		_info.AddAccessor<PhysicsComponent, float>("rotationalInertia",
			[](PhysicsComponent& _c) { Float _m, _i; FVector2 _center; _c.GetMassData(_m, _i, _center); return static_cast<float>(_i); },
			[](PhysicsComponent& _c, const float& _v) { Float _m, _i; FVector2 _center; _c.GetMassData(_m, _i, _center); _c.SetMassData(_m, Float(std::max(0.f, _v)), _center); });
		_info.AddAccessor<PhysicsComponent, FVector2>("centerOfMass",
			[](PhysicsComponent& _c) { Float _m, _i; FVector2 _center; _c.GetMassData(_m, _i, _center); return _center; },
			[](PhysicsComponent& _c, const FVector2& _v) { Float _m, _i; FVector2 _center; _c.GetMassData(_m, _i, _center); _c.SetMassData(_m, _i, _v); });

		_info.AddAccessor<PhysicsComponent, bool>("fixedRotation",
			[](PhysicsComponent& _c) { return _c.IsFixedRotation(); },
			[](PhysicsComponent& _c, const bool& _v) { _c.SetFixedRotation(_v); });
		_info.AddAccessor<PhysicsComponent, bool>("sleepingAllowed",
			[](PhysicsComponent& _c) { return _c.IsSleepingAllowed(); },
			[](PhysicsComponent& _c, const bool& _v) { _c.EnableSleeping(_v); });
		_info.AddAccessor<PhysicsComponent, float>("sleepThreshold",
			[](PhysicsComponent& _c) { return static_cast<float>(_c.GetSleepThreshold()); },
			[](PhysicsComponent& _c, const float& _v) { _c.SetSleepThreshold(_v); });
		_info.AddAccessor<PhysicsComponent, bool>("awake",
			[](PhysicsComponent& _c) { return _c.IsAwake(); },
			[](PhysicsComponent& _c, const bool& _v) { _c.SetAwake(_v); });
		_info.AddAccessor<PhysicsComponent, bool>("enabled",	// a disabled body is out of the simulation
			[](PhysicsComponent& _c) { return _c.IsEnabled(); },
			[](PhysicsComponent& _c, const bool& _v) { if (_v) _c.Enable(); else _c.Disable(); });
		_info.AddAccessor<PhysicsComponent, bool>("contactEvents",
			[](PhysicsComponent& _c) { return _c.AreContactEventsEnabled(); },
			[](PhysicsComponent& _c, const bool& _v) { _c.EnableContactEvents(_v); });
		_info.AddAccessor<PhysicsComponent, bool>("bullet",
			[](PhysicsComponent& _c) { return _c.IsBullet(); },
			[](PhysicsComponent& _c, const bool& _v) { _c.SetBullet(_v); });

		// Collision filter (SetFilter takes the three together).
		_info.AddAccessor<PhysicsComponent, int>("filterCategory",
			[](PhysicsComponent& _c) { CollisionChannel _cat, _mask; int32_t _group; _c.GetFilter(_cat, _mask, _group); return ChannelToInt(_cat); },
			[](PhysicsComponent& _c, const int& _v) { CollisionChannel _cat, _mask; int32_t _group; _c.GetFilter(_cat, _mask, _group); _c.SetFilter(IntToChannel(_v), _mask, _group); });
		_info.SetLastPropertyType(PropertyType::Flags, ChannelNames());
		_info.AddAccessor<PhysicsComponent, int>("filterMask",
			[](PhysicsComponent& _c) { CollisionChannel _cat, _mask; int32_t _group; _c.GetFilter(_cat, _mask, _group); return ChannelToInt(_mask); },
			[](PhysicsComponent& _c, const int& _v) { CollisionChannel _cat, _mask; int32_t _group; _c.GetFilter(_cat, _mask, _group); _c.SetFilter(_cat, IntToChannel(_v), _group); });
		_info.SetLastPropertyType(PropertyType::Flags, ChannelNames());
		_info.AddAccessor<PhysicsComponent, int>("filterGroup",	// bodies of the same negative group never collide, of the same positive group always do
			[](PhysicsComponent& _c) { CollisionChannel _cat, _mask; int32_t _group; _c.GetFilter(_cat, _mask, _group); return static_cast<int>(_group); },
			[](PhysicsComponent& _c, const int& _v) { CollisionChannel _cat, _mask; int32_t _group; _c.GetFilter(_cat, _mask, _group); _c.SetFilter(_cat, _mask, static_cast<int32_t>(_v)); });
		AddDebugProperty<PhysicsComponent>(_info);	// cyan outline of the box2d shape
	}

	{
		ClassInfo& _info = Describe("RigidbodyComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<RigidbodyComponent>(); };
		_info.AddAccessor<RigidbodyComponent, float>("mass",
			[](RigidbodyComponent& _c) { return static_cast<float>(_c.GetMass()); },
			[](RigidbodyComponent& _c, const float& _v) { _c.SetMass(_v); });
		_info.AddProperty<RigidbodyComponent, Bool>("useGravity", &RigidbodyComponent::useGravity);
		_info.AddProperty<RigidbodyComponent, Bool>("isKinematic", &RigidbodyComponent::isKinematic);
		_info.AddProperty<RigidbodyComponent, Float>("gravity", &RigidbodyComponent::gravity);
		_info.AddProperty<RigidbodyComponent, Bool>("freezeRotation", &RigidbodyComponent::freezeRotation);
		_info.AddProperty<RigidbodyComponent, Bool>("freezeMovementX", &RigidbodyComponent::freezeMovementX);
		_info.AddProperty<RigidbodyComponent, Bool>("freezeMovementY", &RigidbodyComponent::freezeMovementY);
		_info.AddProperty<RigidbodyComponent, Float>("linearDamping", &RigidbodyComponent::linearDamping);
		_info.AddProperty<RigidbodyComponent, Float>("angularDamping", &RigidbodyComponent::angularDamping);
		_info.AddProperty<RigidbodyComponent, Float>("restitution", &RigidbodyComponent::restitution, 0.f, 1.f);
		_info.AddProperty<RigidbodyComponent, Float>("staticFriction", &RigidbodyComponent::staticFriction);
		_info.AddProperty<RigidbodyComponent, Float>("dynamicFriction", &RigidbodyComponent::dynamicFriction);
		_info.AddProperty<RigidbodyComponent, Float>("maxLinearVelocity", &RigidbodyComponent::maxLinearVelocity);
		_info.AddProperty<RigidbodyComponent, Float>("maxAngularVelocity", &RigidbodyComponent::maxAngularVelocity);
	}

	{
		ClassInfo& _info = Describe("CameraComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<CameraComponent>(); };
		_info.AddProperty<CameraComponent, Bool>("attachedToOwner", &CameraComponent::attachedToOwner);
		_info.AddProperty<CameraComponent, Bool>("freezePostition", &CameraComponent::freezePostition);
		_info.AddProperty<CameraComponent, Bool>("freezeRotation", &CameraComponent::freezeRotation);
		_info.AddAccessor<CameraComponent, FVector2>("size",
			[](CameraComponent& _c) { return FVector2(_c.GetView()->getSize().x, _c.GetView()->getSize().y); },
			[](CameraComponent& _c, const FVector2& _v) { _c.SetSize(_v); });
		AddDebugProperty<CameraComponent>(_info);	// yellow outline of the area it shows
	}

	{
		ClassInfo& _info = Describe("LabelComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<LabelComponent>(TextData()); };
		AddLabelProperties<LabelComponent>(_info);
	}

	{
		ClassInfo& _info = Describe("AudioListenerComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<AudioListenerComponent>(); };
		_info.AddAccessor<AudioListenerComponent, bool>("topView",
			[](AudioListenerComponent& _c) { return static_cast<bool>(_c.IsTopView()); },
			[](AudioListenerComponent& _c, const bool& _v) { _c.SetTopView(Bool(_v)); });
		_info.AddAccessor<AudioListenerComponent, bool>("listening",
			[](AudioListenerComponent& _c) { return static_cast<bool>(_c.AudioListenerComponent::IsActive()); },
			[](AudioListenerComponent& _c, const bool& _v) { _c.AudioListenerComponent::SetActive(Bool(_v)); });

		// The hearing cone (SetCone takes the three together): sound inside the inner angle is heard fully, outside the outer angle at outerGain.
		_info.AddAccessor<AudioListenerComponent, float>("coneInnerAngle",		// degrees
			[](AudioListenerComponent& _c) { return _c.GetCone().innerAngle.asDegrees(); },
			[](AudioListenerComponent& _c, const float& _v) { sf::Listener::Cone _cone = _c.GetCone(); _cone.innerAngle = sf::degrees(std::clamp(_v, 0.f, 360.f)); _c.SetCone(_cone); }, 0.f, 360.f);
		_info.AddAccessor<AudioListenerComponent, float>("coneOuterAngle",		// degrees
			[](AudioListenerComponent& _c) { return _c.GetCone().outerAngle.asDegrees(); },
			[](AudioListenerComponent& _c, const float& _v) { sf::Listener::Cone _cone = _c.GetCone(); _cone.outerAngle = sf::degrees(std::clamp(_v, 0.f, 360.f)); _c.SetCone(_cone); }, 0.f, 360.f);
		_info.AddAccessor<AudioListenerComponent, float>("coneOuterGain",		// 0 silent, 1 full volume
			[](AudioListenerComponent& _c) { return _c.GetCone().outerGain; },
			[](AudioListenerComponent& _c, const float& _v) { sf::Listener::Cone _cone = _c.GetCone(); _cone.outerGain = std::clamp(_v, 0.f, 1.f); _c.SetCone(_cone); }, 0.f, 1.f);
		AddDebugProperty<AudioListenerComponent>(_info);	// the cone and its direction
	}

	{
		ClassInfo& _info = Describe("SpatialSoundComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<SpatialSoundComponent>(); };
		_info.AddAccessor<SpatialSoundComponent, std::string>("sound",
			[](SpatialSoundComponent& _c) { return _c.soundFile.StdString().empty() ? std::string() : _c.soundFile.StdString() + (_c.soundType == AudioExtensionType::WAV ? ".wav" : ".mp3"); },
			[](SpatialSoundComponent& _c, const std::string& _v)
			{
				const bool _hasExtension = _v.size() > 4 && _v[_v.size() - 4] == '.';
				const bool _wav = _hasExtension && (_v.compare(_v.size() - 4, 4, ".wav") == 0 || _v.compare(_v.size() - 4, 4, ".WAV") == 0);
				_c.SetSoundFile(String(_hasExtension ? _v.substr(0, _v.size() - 4) : _v), _wav ? AudioExtensionType::WAV : AudioExtensionType::MP3);
			});
		_info.SetLastPropertyType(PropertyType::Sound);
		_info.AddAccessor<SpatialSoundComponent, float>("volume",
			[](SpatialSoundComponent& _c) { return static_cast<float>(_c.GetVolume()); },
			[](SpatialSoundComponent& _c, const float& _v) { _c.SetVolume(_v); }, 0.f, 100.f);
		_info.AddAccessor<SpatialSoundComponent, bool>("loop",
			[](SpatialSoundComponent& _c) { return static_cast<bool>(_c.IsLooping()); },
			[](SpatialSoundComponent& _c, const bool& _v) { _c.SetLoop(Bool(_v)); });
		_info.AddAccessor<SpatialSoundComponent, float>("pitch",
			[](SpatialSoundComponent& _c) { return static_cast<float>(_c.GetPitch()); },
			[](SpatialSoundComponent& _c, const float& _v) { _c.SetPitch(std::max(0.01f, _v)); });
		_info.AddAccessor<SpatialSoundComponent, float>("attenuation",
			[](SpatialSoundComponent& _c) { return static_cast<float>(_c.GetAttenuation()); },
			[](SpatialSoundComponent& _c, const float& _v) { _c.SetAttenuation(_v); });
		_info.AddProperty<SpatialSoundComponent, Bool>("isTopDown", &SpatialSoundComponent::isTopDown);
		_info.AddProperty<SpatialSoundComponent, Bool>("playOnBeginPlay", &SpatialSoundComponent::playOnBeginPlay);
	}

	{
		ClassInfo& _info = Describe("AnimationComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<AnimationComponent>(); };
		AddAnimationProperties<AnimationComponent>(_info);
	}

	{
		ClassInfo& _info = Describe("AnimationComponentSM", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<AnimationComponentSM>(); };
		AddAnimationProperties<AnimationComponentSM>(_info);
	}

	{
		ClassInfo& _info = Describe("InputComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<InputComponent>(); };
		_info.AddProperty<InputComponent, Bool>("enabled", &InputComponent::enabled);
		_info.AddAccessor<InputComponent, std::string>("bindings",
			[](InputComponent& _c) { return InputComponent::EncodeBindings(_c.bindings); },
			[](InputComponent& _c, const std::string& _v) { _c.bindings = InputComponent::DecodeBindings(_v); _c.ApplyBindings(); });
		_info.SetLastPropertyType(PropertyType::InputBindings);
	}

	// ================= Settings (the Settings window of the editor) =================
	{
		ClassInfo& _info = Describe("LoggerSettings", "IGameObject");
		AddLoggerFlag(_info, "verboseLogging", Logger::verboseLogging);
		AddLoggerFlag(_info, "veryVerboseLogging", Logger::veryVerboseLogging);
		AddLoggerFlag(_info, "colorText", Logger::canColorText);
		AddLoggerFlag(_info, "playSounds", Logger::canPlaySound);
		AddLoggerFlag(_info, "messageSound", Logger::canPlayMsgSound);
		AddLoggerFlag(_info, "warningSound", Logger::canPlayWarningSound);
		AddLoggerFlag(_info, "errorSound", Logger::canPlayErrorSound);
		AddLoggerFlag(_info, "fatalSound", Logger::canPlayFatalSound);
	}

	{
		ClassInfo& _info = Describe("PhysicsSettings", "IGameObject");
		AddLive(_info, "gravity", &PhysicsSettings::gravity);
		AddLive(_info, "subSteps", &PhysicsSettings::subSteps, 1.f, 32.f);
		AddLive(_info, "sleeping", &PhysicsSettings::sleeping);
		AddLive(_info, "continuous", &PhysicsSettings::continuous);
		AddLive(_info, "warmStarting", &PhysicsSettings::warmStarting);
		AddLive(_info, "maxLinearSpeed", &PhysicsSettings::maxLinearSpeed, 1.f, 2000.f);
		AddLive(_info, "restitutionThreshold", &PhysicsSettings::restitutionThreshold, 0.f, 20.f);
		AddLive(_info, "hitEventThreshold", &PhysicsSettings::hitEventThreshold, 0.f, 20.f);
		AddLive(_info, "contactHertz", &PhysicsSettings::contactHertz, 1.f, 120.f);
		AddLive(_info, "contactDampingRatio", &PhysicsSettings::contactDampingRatio, 0.f, 50.f);
		AddLive(_info, "maxContactPushSpeed", &PhysicsSettings::maxContactPushSpeed, 0.f, 20.f);
	}

	{
		ClassInfo& _info = Describe("TimeSettings", "IGameObject");
		AddLive(_info, "timeScale", &TimeSettings::timeScale, 0.f, 5.f);
		AddLive(_info, "maxFrameRate", &TimeSettings::maxFrameRate, 0.f, 1000.f);	// 0 = unlimited
	}

	{
		ClassInfo& _info = Describe("WindowSettings", "IGameObject");
		AddLive(_info, "verticalSync", &WindowSettings::verticalSync);
	}

	{
		ClassInfo& _info = Describe("AudioSettings", "IGameObject");
		AddLive(_info, "globalVolume", &AudioSettings::globalVolume, 0.f, 100.f);
	}

	{
		ClassInfo& _info = Describe("ShipSettings", "IGameObject");
		_info.AddProperty<ShipSettings, std::string>("startScene", &ShipSettings::startScene);
		_info.SetLastPropertyType(PropertyType::Scene);
		_info.AddProperty<ShipSettings, std::string>("gameName", &ShipSettings::gameName);
		_info.AddProperty<ShipSettings, int>("windowMode", &ShipSettings::windowMode, 0.f, 2.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Borderless (desktop size)", "Windowed", "Fullscreen" });
		_info.AddProperty<ShipSettings, int>("windowWidth", &ShipSettings::windowWidth, 320.f, 7680.f);
		_info.AddProperty<ShipSettings, int>("windowHeight", &ShipSettings::windowHeight, 240.f, 4320.f);
		_info.AddProperty<ShipSettings, bool>("showSplash", &ShipSettings::showSplash);
	}

	// ================= HUD widgets =================
	{
		ClassInfo& _info = Describe("ButtonWidget", "Widget");
		_info.widgetFactory = [](Level* _level) -> Widget*
		{
			return _level->GetHUD().CreateWidget<ButtonWidget>(_level, RectangleShapeData());
		};
		_info.AddProperty<ButtonWidget, Int>("action", &ButtonWidget::action, 0.f, 2.f);	// 0 nothing, 1 open scene, 2 quit
		_info.SetLastPropertyType(PropertyType::Enum, { "Nothing", "Open scene", "Quit game" });
		_info.AddProperty<ButtonWidget, String>("targetScene", &ButtonWidget::targetScene);
	}

	// ================= Widget components =================
	{
		ClassInfo& _info = Describe("AnimationWidgetComponent", "WidgetComponent");
		_info.widgetComponentFactory = [](Widget* _owner) -> WidgetComponent*
		{
			if (!_owner->GetComponent<ImageWidgetComponent>())
			{
				LOG_WARNING("AnimationWidgetComponent needs an ImageWidgetComponent on the same widget");
				return nullptr;
			}
			return _owner->AddComponent<AnimationWidgetComponent>();
		};
		AddAnimationProperties<AnimationWidgetComponent>(_info);
	}

	{
		ClassInfo& _info = Describe("ImageWidgetComponent", "WidgetComponent");
		_info.widgetComponentFactory = [](Widget* _owner) -> WidgetComponent* { return _owner->AddComponent<ImageWidgetComponent>(RectangleShapeData()); };
		AddShapeProperties<ImageWidgetComponent>(_info);
	}

	{
		ClassInfo& _info = Describe("LabelWidgetComponent", "WidgetComponent");
		_info.widgetComponentFactory = [](Widget* _owner) -> WidgetComponent* { return _owner->AddComponent<LabelWidgetComponent>(TextData()); };
		AddLabelProperties<LabelWidgetComponent>(_info);
	}

	{
		ClassInfo& _info = Describe("ButtonWidgetComponent", "WidgetComponent");
		_info.AddProperty<ButtonWidgetComponent, EventList>("onPress", &ButtonWidgetComponent::onPressCalls);
		_info.AddProperty<ButtonWidgetComponent, EventList>("onRelease", &ButtonWidgetComponent::onReleaseCalls);
		_info.AddProperty<ButtonWidgetComponent, EventList>("onHover", &ButtonWidgetComponent::onHoverCalls);
		_info.AddProperty<ButtonWidgetComponent, EventList>("onUnhover", &ButtonWidgetComponent::onUnhoverCalls);
		_info.widgetComponentFactory = [](Widget* _owner) -> WidgetComponent*
		{
			if (!_owner->GetComponent<ImageWidgetComponent>())
			{
				LOG_WARNING("ButtonWidgetComponent needs an ImageWidgetComponent on the same widget");
				return nullptr;
			}
			return _owner->AddComponent<ButtonWidgetComponent>();
		};
	}

	// ================= Functions the editor can call (event lists) =================
	{
		ClassInfo& _info = Describe("IGameObject", "");
		_info.AddFunction<IGameObject, bool>("SetActive", [](IGameObject& _object, const bool& _active) { _object.SetActive(Bool(_active)); });
	}

	{
		ClassInfo& _info = Describe("Actor", "IGameObject");
		_info.AddFunction<Actor>("Destroy", [](Actor& _actor) { _actor.Destroy(); });
		_info.AddFunction<Actor, FVector2>("SetPosition", [](Actor& _actor, const FVector2& _position) { _actor.SetPosition(_position); });
		_info.AddFunction<Actor, float>("SetRotation", [](Actor& _actor, const float& _degrees) { _actor.SetRotation(Angle::FromDegrees(_degrees)); });
		_info.AddFunction<Actor, FVector2>("SetScale", [](Actor& _actor, const FVector2& _scale) { _actor.SetScale(_scale); });
	}

	{
		ClassInfo& _info = Describe("Widget", "IGameObject");
		_info.AddFunction<Widget>("Destroy", [](Widget& _widget) { _widget.Destroy(); });
		_info.AddFunction<Widget, FVector2>("SetPosition", [](Widget& _widget, const FVector2& _position) { _widget.SetPosition(_position); });
		_info.AddFunction<Widget, float>("SetRotation", [](Widget& _widget, const float& _degrees) { _widget.SetRotation(Angle::FromDegrees(_degrees)); });
		_info.AddFunction<Widget, FVector2>("SetScale", [](Widget& _widget, const FVector2& _scale) { _widget.SetScale(_scale); });
	}

	AddAnimationFunctions<AnimationComponent>(Describe("AnimationComponent", "Component"));
	AddAnimationFunctions<AnimationComponentSM>(Describe("AnimationComponentSM", "Component"));
	AddAnimationFunctions<AnimationWidgetComponent>(Describe("AnimationWidgetComponent", "WidgetComponent"));
	Describe("AnimationComponent", "Component").AddFunction<AnimationComponent, std::string>("Play", [](AnimationComponent& _c, const std::string& _name) { _c.SetCurrentAnimation(String(_name)); _c.StartAnimation(); });
	Describe("AnimationWidgetComponent", "WidgetComponent").AddFunction<AnimationWidgetComponent, std::string>("Play", [](AnimationWidgetComponent& _c, const std::string& _name) { _c.SetCurrentAnimation(String(_name)); _c.StartAnimation(); });

	Describe("LabelComponent", "Component").AddFunction<LabelComponent, std::string>("SetText", [](LabelComponent& _c, const std::string& _text) { _c.SetString(String(_text)); });
	Describe("LabelWidgetComponent", "WidgetComponent").AddFunction<LabelWidgetComponent, std::string>("SetText", [](LabelWidgetComponent& _c, const std::string& _text) { _c.SetString(String(_text)); });

	{
		ClassInfo& _info = Describe("SpatialSoundComponent", "Component");
		_info.AddFunction<SpatialSoundComponent>("Play", [](SpatialSoundComponent& _c) { _c.Play(); });
		_info.AddFunction<SpatialSoundComponent>("Pause", [](SpatialSoundComponent& _c) { _c.Pause(); });
		_info.AddFunction<SpatialSoundComponent>("Stop", [](SpatialSoundComponent& _c) { _c.Stop(); });
	}

	Describe("CameraComponent", "Component").AddFunction<CameraComponent>("MakeCurrent", [](CameraComponent& _c) { _c.SetCurrent(); });

	{
		ClassInfo& _info = Describe("PhysicsComponent", "Component");
		_info.AddFunction<PhysicsComponent, FVector2>("SetLinearVelocity", [](PhysicsComponent& _c, const FVector2& _velocity) { _c.SetLinearVelocity(_velocity); });
		_info.AddFunction<PhysicsComponent, float>("SetAngularVelocity", [](PhysicsComponent& _c, const float& _velocity) { _c.SetAngularVelocity(Float(_velocity)); });
	}

	// "Game" is the target of the functions that are not about one object.
	{
		ClassInfo& _info = Describe("Game", "");
		_info.AddFunction<IGameObject, std::string>("OpenScene", [](IGameObject&, const std::string& _scene)
		{
			if (Engine* _engine = Reflection::CurrentEngine()) Scene::Open(*_engine, _scene);
		});
		_info.SetLastFunctionArgumentType(PropertyType::Scene);
		_info.AddFunction<IGameObject>("Quit", [](IGameObject&)
		{
			if (Engine* _engine = Reflection::CurrentEngine()) _engine->QuitGame();
		});
	}

	// ================= Network =================
	{
		ClassInfo& _info = Describe("NetworkSettings", "IGameObject");
		AddLive(_info, "port", &NetworkSettings::port, 1.f, 65535.f);
		AddLive(_info, "address", &NetworkSettings::address);
		AddLive(_info, "playerName", &NetworkSettings::playerName);
		AddLive(_info, "maxClients", &NetworkSettings::maxClients, 1.f, 64.f);
		AddLive(_info, "timeout", &NetworkSettings::timeout, 1.f, 120.f);
		AddLive(_info, "discoveryPort", &NetworkSettings::discoveryPort, 1.f, 65535.f);
		AddLive(_info, "lanVisible", &NetworkSettings::lanVisible);
	}

	{
		ClassInfo& _info = Describe("NetworkSyncComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<NetworkSyncComponent>(); };
		_info.AddProperty<NetworkSyncComponent, Int>("networkId", &NetworkSyncComponent::networkId);
		_info.AddProperty<NetworkSyncComponent, Bool>("syncPosition", &NetworkSyncComponent::syncPosition);
		_info.AddProperty<NetworkSyncComponent, Bool>("syncRotation", &NetworkSyncComponent::syncRotation);
		_info.AddProperty<NetworkSyncComponent, Bool>("syncScale", &NetworkSyncComponent::syncScale);
		_info.AddProperty<NetworkSyncComponent, Float>("sendRate", &NetworkSyncComponent::sendRate, 1.f, 60.f);
		_info.AddProperty<NetworkSyncComponent, Bool>("smoothing", &NetworkSyncComponent::smoothing);
		_info.AddProperty<NetworkSyncComponent, Bool>("reliable", &NetworkSyncComponent::reliable);
		_info.AddProperty<NetworkSyncComponent, String>("properties", &NetworkSyncComponent::properties);

		// Calls a KFUNCTION of this actor (or of one of its components) on other machines: the argument is the function's name.
		_info.AddFunction<NetworkSyncComponent, std::string>("RpcAll", [](NetworkSyncComponent& _c, const std::string& _function) { _c.CallRPC(_function, NetTarget::All); });
		_info.AddFunction<NetworkSyncComponent, std::string>("RpcOthers", [](NetworkSyncComponent& _c, const std::string& _function) { _c.CallRPC(_function, NetTarget::Others); });
		_info.AddFunction<NetworkSyncComponent, std::string>("RpcHost", [](NetworkSyncComponent& _c, const std::string& _function) { _c.CallRPC(_function, NetTarget::Host); });
		_info.AddFunction<NetworkSyncComponent>("Despawn", [](NetworkSyncComponent& _c) { _c.GetWorld()->GetNetwork()->Despawn(_c.GetNetworkId()); });
	}

	{
		ClassInfo& _info = Describe("NetworkEventsComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<NetworkEventsComponent>(); };
		_info.AddProperty<NetworkEventsComponent, EventList>("onHosted", &NetworkEventsComponent::onHosted);
		_info.AddProperty<NetworkEventsComponent, EventList>("onConnected", &NetworkEventsComponent::onConnected);
		_info.AddProperty<NetworkEventsComponent, EventList>("onConnectionFailed", &NetworkEventsComponent::onConnectionFailed);
		_info.AddProperty<NetworkEventsComponent, EventList>("onDisconnected", &NetworkEventsComponent::onDisconnected);
		_info.AddProperty<NetworkEventsComponent, EventList>("onPlayerJoined", &NetworkEventsComponent::onPlayerJoined);
		_info.AddProperty<NetworkEventsComponent, EventList>("onPlayerLeft", &NetworkEventsComponent::onPlayerLeft);
		_info.AddProperty<NetworkEventsComponent, EventList>("onSharedChanged", &NetworkEventsComponent::onSharedChanged);
		_info.AddProperty<NetworkEventsComponent, EventList>("onLanGameFound", &NetworkEventsComponent::onLanGameFound);
		_info.AddAccessor<NetworkEventsComponent, std::string>("messages",
			[](NetworkEventsComponent& _c) { return InputComponent::EncodeBindings(_c.messages); },
			[](NetworkEventsComponent& _c, const std::string& _v) { _c.messages = InputComponent::DecodeBindings(_v); _c.ApplyMessages(); });
		_info.SetLastPropertyType(PropertyType::MessageBindings);
	}

	// Network functions of the "Game" target.
	{
		ClassInfo& _info = Describe("Game", "");
		const auto _net = []() { return Reflection::CurrentEngine()->GetNetwork(); };
		_info.AddFunction<IGameObject>("Host", [_net](IGameObject&) { _net()->Host(); });
		_info.AddFunction<IGameObject>("Join", [_net](IGameObject&) { _net()->Join(); });
		_info.AddFunction<IGameObject, std::string>("JoinAddress", [_net](IGameObject&, const std::string& _address) { _net()->Join(_address); });
		_info.AddFunction<IGameObject>("Disconnect", [_net](IGameObject&) { _net()->Disconnect(); });
		_info.AddFunction<IGameObject, std::string>("SendMessage", [_net](IGameObject&, const std::string& _name) { _net()->Send(_name); });
		_info.AddFunction<IGameObject, std::string>("SpawnNetworked", [_net](IGameObject&, const std::string& _className) { _net()->Spawn(_className); });
		_info.AddFunction<IGameObject>("SearchLan", [_net](IGameObject&) { _net()->SearchLan(); });
		_info.AddFunction<IGameObject>("JoinFirstLanGame", [_net](IGameObject&)
		{
			if (_net()->GetLanGames().empty()) LOG_WARNING("Network: no LAN game found yet (call SearchLan first)");
			else _net()->Join(_net()->GetLanGames().front());
		});
		_info.AddFunction<IGameObject, std::string>("SetPlayerName", [_net](IGameObject&, const std::string& _name) { _net()->SetPlayerName(_name); });
		_info.AddFunction<IGameObject, std::string>("AddSharedOne", [_net](IGameObject&, const std::string& _key) { _net()->AddShared(_key, 1.f); });
	}
}
