#pragma once
#include "Reflection.h"
#include "Actors/CameraActor.h"
#include "Actors/EmptyActor.h"
#include "Components/AnimationComponent.h"
#include "Components/AnimationComponentSM.h"
#include "Components/InputComponent.h"
#include "Components/AudioListenerComponent.h"
#include "Components/CameraComponent.h"
#include "Components/LabelComponent.h"
#include "Components/NetworkEventsComponent.h"
#include "Components/NetworkSyncComponent.h"
#include "Network/NetworkManager.h"
#include "Components/PhysicsComponent.h"
#include "Components/ParticleComponent.h"
#include "Components/StainComponent.h"
#include "Components/BillboardComponent.h"
#include "Navigation/NavigationComponents.h"
#include "Components/PostProcessComponent.h"
#include "Components/LightComponents.h"
#include "Components/GameplayTagComponent.h"
#include "Components/SpatialSoundComponent.h"
#include "Components/SpriteComponent.h"
#include "Core/Settings.h"
#include "Scene/Scene.h"
#include "UI/Widgets/ButtonWidget.h"
#include "UI/Widgets/BasicWidgets.h"
#include "UI/Widgets/LayoutWidgets.h"
#include "UI/Widgets/InputWidgets.h"
#include "UI/Widgets/WidgetComponents/AnimationWidgetComponent.h"
#include "UI/Widgets/WidgetComponents/ButtonWidgetComponent.h"
#include "UI/Widgets/WidgetComponents/ImageWidgetComponent.h"
#include "UI/Widgets/WidgetComponents/LabelWidgetComponent.h"
#include "UI/Diegetic/DiegeticImageWidget.h"
#include "UI/Diegetic/DiegeticLabelWidget.h"
#include "UI/Diegetic/DiegeticButtonWidget.h"

// Helpers and registration steps of EngineClasses*.cpp (the classes of the engine are described in several files, in this order).
namespace Krampus::EngineClassHelpers
{
	inline std::string TextureToString(ShapeObject& _shape);
	inline std::string SplitTexture(const std::string& _value, TextureExtensionType& _type);

	inline ClassInfo& Describe(const char* _name, const char* _parent)
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

	// The "shape" property of the components that have one: 0 rectangle, 1 circle, 2 polygon (saved in scenes: never reorder).
	inline int ShapeToInt(const ShapeType& _type)
	{
		return _type == ShapeType::Circle ? 1 : (_type == ShapeType::Polygon ? 2 : 0);
	}

	inline ShapeType IntToShape(const int& _value)
	{
		return _value == 1 ? ShapeType::Circle : (_value == 2 ? ShapeType::Polygon : ShapeType::Rectangle);
	}

	inline std::vector<std::string> ShapeNames()
	{
		return { "Rectangle", "Circle", "Polygon" };
	}

	// Sprite and ImageWidgetComponent have the same shape API.
	template <typename C>
	void AddShapeProperties(ClassInfo& _info)
	{
		// First: size and radius only make sense once the shape type is right when a scene is loaded.
		_info.AddAccessor<C, int>("shape",		// 0 rectangle, 1 circle, 2 polygon
			[](C& _c) { return ShapeToInt(_c.GetShapeType()); },
			[](C& _c, const int& _v) { _c.GetShapeObject()->SetShapeType(IntToShape(_v)); }, 0.f, 2.f);
		_info.SetLastPropertyType(PropertyType::Enum, ShapeNames());
		_info.AddAccessor<C, std::string>("polygon",
			[](C& _c) { return _c.GetShapeObject()->GetPolygonName(); },
			[](C& _c, const std::string& _v) { if (_c.GetShapeType() == ShapeType::Polygon) _c.GetShapeObject()->SetPolygon(_v); });
		_info.SetLastPropertyType(PropertyType::Shape);
		_info.SetLastPropertyVisible<C>([](C& _c) { return _c.GetShapeType() == ShapeType::Polygon; });	// polygon
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
			[](C& _c, const bool& _v) { _c.GetShapeObject()->GetTextureRef().SetSmooth(_v); });
		_info.AddAccessor<C, bool>("isRepeated",
			[](C& _c) { return static_cast<bool>(_c.GetShapeObject()->GetTextureRef().IsRepeated()); },
			[](C& _c, const bool& _v) { _c.GetShapeObject()->GetTextureRef().SetRepeated(_v); });
		// Multiplies the picture (white: as it is); a tint for placeholder textures.
		_info.AddAccessor<C, Color>("color",
			[](C& _c) { return _c.GetShapeObject()->GetShape()->GetFillColor(); },
			[](C& _c, const Color& _v) { _c.GetShapeObject()->GetShape()->SetFillColor(_v); });
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
			[](C& _c) { return _c.startAnimation; },
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
			[](C& _c) { return _c.GetFontFileName() + (_c.GetFontType() == FontExtensionType::TTF ? ".ttf" : ".otf"); },
			[](C& _c, const std::string& _v)
			{
				const bool _ttf = _v.size() > 4 && (_v.compare(_v.size() - 4, 4, ".ttf") == 0 || _v.compare(_v.size() - 4, 4, ".TTF") == 0);
				const bool _hasExtension = _v.size() > 4 && _v[_v.size() - 4] == '.';
				_c.SetFont(String(_hasExtension ? _v.substr(0, _v.size() - 4) : _v), _ttf ? FontExtensionType::TTF : FontExtensionType::OTF);
			});
		_info.SetLastPropertyType(PropertyType::Font);
		_info.AddAccessor<C, std::string>("text",
			[](C& _c) { return _c.GetString(); },
			[](C& _c, const std::string& _v) { _c.SetString(_v); });
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
	inline void AddLoggerFlag(ClassInfo& _info, const char* _name, Bool& _field)
	{
		Bool* _pointer = &_field;
		_info.AddAccessor<LoggerSettings, bool>(_name,
			[_pointer](LoggerSettings&) { return static_cast<bool>(*_pointer); },
			[_pointer](LoggerSettings&, const bool& _v) { *_pointer = _v; });
	}

	inline const char* ExtensionOf(const TextureExtensionType& _type)
	{
		static const char* _names[] = { "png", "jpg", "gif", "bmp", "tga", "psd", "hdr", "pic", "pnm" };
		return _names[static_cast<int>(_type)];
	}

	// "Folder/Name.png"
	inline std::string TextureToString(ShapeObject& _shape)
	{
		return _shape.GetTexturePath() + "." + ExtensionOf(_shape.GetTextureType());
	}

	// "Folder/Name.jpg" -> "Folder/Name" and JPG. No extension: PNG.
	inline std::string SplitTexture(const std::string& _value, TextureExtensionType& _type)
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
	inline const std::vector<std::string>& ChannelNames()
	{
		static const std::vector<std::string> _names =
		{
			"Wall", "Player", "Enemy", "Projectile", "Trigger", "StaticWorld", "DynamicWorld", "Destructible", "Terrain", "Water", "Foliage",
			"NPC", "Boss", "Vehicle", "Companion", "PlayerProjectile", "EnemyProjectile", "AreaOfEffect", "PhysicsProp", "Ragdoll", "Pickup",
			"Shield", "Interactable", "Vision", "AudioTrigger", "Checkpoint", "KillZone", "Camera", "Cursor", "NavMesh", "HitboxHead", "HitboxLimb"
		};
		return _names;
	}

	inline int ChannelToInt(const CollisionChannel& _channel)
	{
		return static_cast<int>(static_cast<std::uint32_t>(static_cast<std::uint64_t>(_channel)));
	}

	inline CollisionChannel IntToChannel(const int& _value)
	{
		return static_cast<CollisionChannel>(static_cast<std::uint64_t>(static_cast<std::uint32_t>(_value)));
	}

	// The shader of what is drawn and its parameter values (hidden: the Inspector draws them in its Material section).
	template <typename C>
	void AddMaterialProperties(ClassInfo& _info)
	{
		_info.AddAccessor<C, std::string>("shader",
			[](C& _c) { return _c.material.shader; },
			[](C& _c, const std::string& _v) { _c.material.shader = _v; });
		_info.SetLastPropertyVisible<C>([](C&) { return false; });
		_info.AddAccessor<C, std::string>("shaderParams",
			[](C& _c) { return _c.material.EncodeValues(); },
			[](C& _c, const std::string& _v) { _c.material.DecodeValues(_v); });
		_info.SetLastPropertyVisible<C>([](C&) { return false; });
		_info.AddFunction<C, std::string>("SetShader", [](C& _c, const std::string& _name) { _c.material.shader = _name; });
		// "name=value" (amount=1, tint=1, 0, 0, 1): one argument only for the event lists.
		_info.AddFunction<C, std::string>("SetShaderParam", [](C& _c, const std::string& _text)
		{
			const size_t _equal = _text.find('=');
			if (_equal == std::string::npos) { LOG_WARNING("SetShaderParam: write name=value, e.g. amount=1"); return; }
			_c.material.SetParam(_text.substr(0, _equal), _text.substr(_equal + 1));
		});
	}

	// A value of ParticleSettings shown in the Inspector (the settings are a struct of the component, not members of it).
	template <typename T>
	void AddParticleProperty(ClassInfo& _info, const char* _name, T ParticleSettings::* _member, const float _min = 0.f, const float _max = 0.f)
	{
		_info.AddAccessor<ParticleComponent, T>(_name,
			[_member](ParticleComponent& _c) { return _c.Settings().*_member; },
			[_member](ParticleComponent& _c, const T& _v) { _c.Settings().*_member = _v; }, _min, _max);
	}

	inline ParticleShape ParticleShapeOf(ParticleComponent& _c)
	{
		return static_cast<ParticleShape>(_c.Settings().shape);
	}

	void RegisterComponentClasses();
	void RegisterEffectClasses();
	void RegisterWidgetClasses();
	void RegisterFunctionClasses();
}
