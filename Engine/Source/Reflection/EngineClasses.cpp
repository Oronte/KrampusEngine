#include "Reflection.h"
#include "Actors/CameraActor.h"
#include "Actors/EmptyActor.h"
#include "Components/AudioListenerComponent.h"
#include "Components/CameraComponent.h"
#include "Components/CollisionComponent.h"
#include "Components/LabelComponent.h"
#include "Components/PhysicsComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/SpatialSoundComponent.h"
#include "Components/SpriteComponent.h"
#include "UI/Widgets/ButtonWidget.h"
#include "UI/Widgets/WidgetComponents/ButtonWidgetComponent.h"
#include "UI/Widgets/WidgetComponents/ImageWidgetComponent.h"
#include "UI/Widgets/WidgetComponents/LabelWidgetComponent.h"

// Everything the Inspector shows for the classes of the engine. The classes themselves stay untouched: values are read
// and written through their public members and Get/Set methods. Classes of the game use KACTOR / KCOMPONENT / KPROPERTY.

using namespace Krampus;

namespace
{
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
			[](C& _c) { return _c.GetShapeObject()->GetTexturePath().StdString(); },
			[](C& _c, const std::string& _v) { _c.SetTexture(_v); });
	}

	// LabelComponent and LabelWidgetComponent have the same text API.
	template <typename C>
	void AddLabelProperties(ClassInfo& _info)
	{
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
		_info.AddAccessor<CollisionComponent, FVector2>("size",
			[](CollisionComponent& _c) { return _c.GetShapeType() == ShapeType::Rectangle ? _c.GetSize() : FVector2(); },
			[](CollisionComponent& _c, const FVector2& _v) { if (_c.GetShapeType() == ShapeType::Rectangle) _c.SetSize(_v); });
		_info.AddAccessor<CollisionComponent, float>("radius",
			[](CollisionComponent& _c) { return _c.GetShapeType() == ShapeType::Circle ? static_cast<float>(_c.GetRadius()) : 0.f; },
			[](CollisionComponent& _c, const float& _v) { if (_c.GetShapeType() == ShapeType::Circle) _c.SetRadius(_v); });
		_info.AddAccessor<CollisionComponent, int>("channel",
			[](CollisionComponent& _c) { return ChannelToInt(_c.channel); },
			[](CollisionComponent& _c, const int& _v) { _c.channel = IntToChannel(_v); });
		_info.AddAccessor<CollisionComponent, int>("mask",
			[](CollisionComponent& _c) { return ChannelToInt(_c.mask); },
			[](CollisionComponent& _c, const int& _v) { _c.mask = IntToChannel(_v); });
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
		_info.AddProperty<SpatialSoundComponent, Bool>("isTopDown", &SpatialSoundComponent::isTopDown);
		_info.AddProperty<SpatialSoundComponent, Bool>("playOnBeginPlay", &SpatialSoundComponent::playOnBeginPlay);
	}

	// ================= HUD widgets =================
	{
		ClassInfo& _info = Describe("ButtonWidget", "Widget");
		_info.widgetFactory = [](Level* _level) -> Widget*
		{
			return _level->GetHUD().CreateWidget<ButtonWidget>(_level, RectangleShapeData());
		};
		_info.AddProperty<ButtonWidget, Int>("action", &ButtonWidget::action, 0.f, 2.f);	// 0 nothing, 1 open scene, 2 quit
		_info.AddProperty<ButtonWidget, String>("targetScene", &ButtonWidget::targetScene);
	}

	// ================= Widget components =================
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
