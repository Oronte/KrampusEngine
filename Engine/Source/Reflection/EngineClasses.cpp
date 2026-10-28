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
#include "Components/PhysicsComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/SpatialSoundComponent.h"
#include "Components/SpriteComponent.h"
#include "Core/Settings.h"
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
		_info.AddAccessor<C, float>("radius",
			[](C& _c) { return _c.GetShapeType() == ShapeType::Circle ? static_cast<float>(_c.GetRadius()) : 0.f; },
			[](C& _c, const float& _v) { if (_c.GetShapeType() == ShapeType::Circle) _c.SetRadius(_v); }, 0.f, 0.f);
		_info.AddAccessor<C, int>("pointCount",
			[](C& _c) { return _c.GetShapeType() == ShapeType::Circle ? static_cast<int>(_c.GetPointCount()) : 0; },
			[](C& _c, const int& _v) { if (_c.GetShapeType() == ShapeType::Circle) _c.SetPointCount(static_cast<size_t>(std::max(3, _v))); });
		_info.AddAccessor<C, std::string>("texture",
			[](C& _c) { return TextureToString(*_c.GetShapeObject()); },
			[](C& _c, const std::string& _v)
			{
				TextureExtensionType _type = TextureExtensionType::PNG;
				const std::string _path = SplitTexture(_v, _type);
				_c.SetTexture(_path, _type);
			});
		_info.SetLastPropertyType(PropertyType::Texture);
	}

	// The three animation components describe one animation (a clip of a sprite sheet) with the same plain values.
	template <typename C>
	void AddAnimationProperties(ClassInfo& _info)
	{
		_info.AddAccessor<C, std::string>("animationName",
			[](C& _c) { return _c.clip.name.StdString(); },
			[](C& _c, const std::string& _v) { _c.clip.name = String(_v); _c.ApplyClip(); });
		_info.AddAccessor<C, int>("frameCount",		// 0 = no animation
			[](C& _c) { return static_cast<int>(_c.clip.frameCount); },
			[](C& _c, const int& _v) { _c.clip.frameCount = Int(std::max(0, _v)); _c.ApplyClip(); });
		_info.AddAccessor<C, float>("duration",		// seconds, whole animation
			[](C& _c) { return static_cast<float>(_c.clip.duration); },
			[](C& _c, const float& _v) { _c.clip.duration = Float(std::max(0.01f, _v)); _c.ApplyClip(); });
		_info.AddAccessor<C, FVector2>("frameOrigin",	// top left corner of the first frame in the texture
			[](C& _c) { return _c.clip.frameOrigin; },
			[](C& _c, const FVector2& _v) { _c.clip.frameOrigin = _v; _c.ApplyClip(); });
		_info.AddAccessor<C, FVector2>("frameSize",
			[](C& _c) { return _c.clip.frameSize; },
			[](C& _c, const FVector2& _v) { _c.clip.frameSize = FVector2(std::max(1.f, _v.x), std::max(1.f, _v.y)); _c.ApplyClip(); });
		_info.AddAccessor<C, int>("direction",		// 0 row, 1 row reversed, 2 column, 3 column reversed
			[](C& _c) { return static_cast<int>(_c.clip.direction); },
			[](C& _c, const int& _v) { _c.clip.direction = Int(std::clamp(_v, 0, 3)); _c.ApplyClip(); }, 0.f, 3.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Row", "Row reversed", "Column", "Column reversed" });
		_info.AddAccessor<C, bool>("loop",
			[](C& _c) { return static_cast<bool>(_c.clip.loop); },
			[](C& _c, const bool& _v) { _c.clip.loop = Bool(_v); _c.ApplyClip(); });
		_info.AddProperty<C, Bool>("playOnBeginPlay", &C::playOnBeginPlay);
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
		_info.AddAccessor<CollisionComponent, float>("radius",
			[](CollisionComponent& _c) { return _c.GetShapeType() == ShapeType::Circle ? static_cast<float>(_c.GetRadius()) : 0.f; },
			[](CollisionComponent& _c, const float& _v) { if (_c.GetShapeType() == ShapeType::Circle) _c.SetRadius(_v); });
		_info.AddAccessor<CollisionComponent, int>("channel",
			[](CollisionComponent& _c) { return ChannelToInt(_c.channel); },
			[](CollisionComponent& _c, const int& _v) { _c.channel = IntToChannel(_v); });
		_info.SetLastPropertyType(PropertyType::Flags, ChannelNames());
		_info.AddAccessor<CollisionComponent, int>("mask",
			[](CollisionComponent& _c) { return ChannelToInt(_c.mask); },
			[](CollisionComponent& _c, const int& _v) { _c.mask = IntToChannel(_v); });
		_info.SetLastPropertyType(PropertyType::Flags, ChannelNames());
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
}
