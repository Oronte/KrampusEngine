#include "EngineClassHelpers.h"

using namespace Krampus;
using namespace Krampus::EngineClassHelpers;

namespace Krampus::EngineClassHelpers
{
	void RegisterComponentClasses()
	{
	// ================= Actor components =================
	{
		ClassInfo& _info = Describe("SpriteComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<SpriteComponent>(RectangleShapeData()); };
		AddShapeProperties<SpriteComponent>(_info);
	}

	{
		ClassInfo& _info = Describe("PhysicsComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component*
		{
			// The body takes the shape of the sprite (and keeps it) when there is one.
			if (_owner->GetComponent<SpriteComponent>())
				return _owner->AddComponent<PhysicsComponent>();
			return _owner->AddComponent<PhysicsComponent>(FVector2(100.f, 100.f));
		};

		// The shape of the body first: changing it computes the mass again from the shape (the mass properties below come after).
		// matchSprite before the shape: while it is on, the saved shape is the sprite's and is not set again (it would turn it off).
		_info.AddAccessor<PhysicsComponent, bool>("matchSprite",	// the shape is always the sprite's (scale included); off: a custom shape
			[](PhysicsComponent& _c) { return _c.IsMatchingSprite(); },
			[](PhysicsComponent& _c, const bool& _v) { if (_v != _c.IsMatchingSprite()) _c.SetMatchSprite(_v); });
		const auto _customShape = [](PhysicsComponent& _c) { return !_c.IsMatchingSprite() || !_c.GetOwner()->GetComponent<SpriteComponent>(); };
		_info.AddAccessor<PhysicsComponent, int>("shape",	// 0 box, 1 circle, 2 polygon
			[](PhysicsComponent& _c) { return ShapeToInt(_c.GetShapeType()); },
			[](PhysicsComponent& _c, const int& _v) { if (!_c.IsMatchingSprite()) _c.SetShapeType(IntToShape(_v)); }, 0.f, 2.f);
		_info.SetLastPropertyType(PropertyType::Enum, ShapeNames());
		_info.SetLastPropertyVisible<PhysicsComponent>(_customShape);
		_info.AddAccessor<PhysicsComponent, std::string>("polygon",
			[](PhysicsComponent& _c) { return _c.GetPolygonName(); },
			[](PhysicsComponent& _c, const std::string& _v) { if (!_c.IsMatchingSprite() && _c.GetShapeType() == ShapeType::Polygon && _v != _c.GetPolygonName()) _c.SetShapePolygon(_v); });
		_info.SetLastPropertyType(PropertyType::Shape);
		_info.SetLastPropertyVisible<PhysicsComponent>([_customShape](PhysicsComponent& _c) { return _customShape(_c) && _c.GetShapeType() == ShapeType::Polygon; });
		_info.AddAccessor<PhysicsComponent, FVector2>("size",	// without the scale of the actor
			[](PhysicsComponent& _c) { return _c.GetBoxSize(); },
			[](PhysicsComponent& _c, const FVector2& _v) { if (!_c.IsMatchingSprite() && _c.GetShapeType() == ShapeType::Rectangle && _v != _c.GetBoxSize()) _c.SetShapeBox(FVector2(std::max(1.f, _v.x), std::max(1.f, _v.y))); });
		_info.SetLastPropertyVisible<PhysicsComponent>([_customShape](PhysicsComponent& _c) { return _customShape(_c) && _c.GetShapeType() == ShapeType::Rectangle; });
		_info.AddAccessor<PhysicsComponent, float>("radius",	// without the scale of the actor
			[](PhysicsComponent& _c) { return static_cast<float>(_c.GetCircleRadius()); },
			[](PhysicsComponent& _c, const float& _v) { if (!_c.IsMatchingSprite() && _c.GetShapeType() == ShapeType::Circle && _v != static_cast<float>(_c.GetCircleRadius())) _c.SetShapeCircle(std::max(1.f, _v)); });
		_info.SetLastPropertyVisible<PhysicsComponent>([_customShape](PhysicsComponent& _c) { return _customShape(_c) && _c.GetShapeType() == ShapeType::Circle; });
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
		_info.AddAccessor<PhysicsComponent, bool>("trigger",	// a trigger does not collide: it tells what enters and leaves it
			[](PhysicsComponent& _c) { return _c.IsTrigger(); },
			[](PhysicsComponent& _c, const bool& _v) { _c.SetTrigger(_v); });
		_info.AddAccessor<PhysicsComponent, bool>("hitEvents",	// onHit when a body hits this one fast enough
			[](PhysicsComponent& _c) { return _c.AreHitEventsEnabled(); },
			[](PhysicsComponent& _c, const bool& _v) { _c.SetHitEventsEnabled(_v); });
		_info.AddProperty<PhysicsComponent, EventList>("onCollisionBegin", &PhysicsComponent::onCollisionBeginCalls);
		_info.AddProperty<PhysicsComponent, EventList>("onCollisionEnd", &PhysicsComponent::onCollisionEndCalls);
		_info.AddProperty<PhysicsComponent, EventList>("onHit", &PhysicsComponent::onHitCalls);
		_info.AddProperty<PhysicsComponent, EventList>("onTriggerBegin", &PhysicsComponent::onTriggerBeginCalls);
		_info.AddProperty<PhysicsComponent, EventList>("onTriggerEnd", &PhysicsComponent::onTriggerEndCalls);

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

	// ---- Shaders
	AddMaterialProperties<SpriteComponent>(Describe("SpriteComponent", "Component"));
	AddMaterialProperties<ParticleComponent>(Describe("ParticleComponent", "Component"));
	AddMaterialProperties<ImageWidgetComponent>(Describe("ImageWidgetComponent", "WidgetComponent"));
	{
		ClassInfo& _info = Describe("PostProcessComponent", "Component");	// a full-screen effect on the world
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<PostProcessComponent>(); };
		_info.AddProperty<PostProcessComponent, int>("priority", &PostProcessComponent::priority, -100.f, 100.f);
		_info.AddProperty<PostProcessComponent, bool>("applyInEditor", &PostProcessComponent::applyInEditor);
		AddMaterialProperties<PostProcessComponent>(_info);
	}
	Describe("PostProcessVolume", "Actor").factory = [](Level* _level) -> Actor*
	{
		EmptyActor* _actor = _level->SpawnActor<EmptyActor>();
		_actor->SetName("PostProcessVolume");
		_actor->AddComponent<PostProcessComponent>()->material.shader = "_internal/Post/Vignette";
		_actor->AddComponent<BillboardComponent>(BillboardIcon::Effect);
		return _actor;
	};

	// ---- 2D lights
	{
		ClassInfo& _info = Describe("PointLightComponent", "Component");	// a round light around the actor
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<PointLightComponent>(); };
		_info.AddProperty<PointLightComponent, Color>("color", &PointLightComponent::color);
		_info.AddProperty<PointLightComponent, float>("intensity", &PointLightComponent::intensity, 0.f, 4.f);
		_info.AddProperty<PointLightComponent, float>("radius", &PointLightComponent::radius, 1.f, 4000.f);
		_info.AddProperty<PointLightComponent, bool>("castShadows", &PointLightComponent::castShadows);
	}
	{
		ClassInfo& _info = Describe("GlobalLightComponent", "Component");	// the light everywhere (ambient)
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<GlobalLightComponent>(); };
		_info.AddProperty<GlobalLightComponent, Color>("color", &GlobalLightComponent::color);
		_info.AddProperty<GlobalLightComponent, float>("intensity", &GlobalLightComponent::intensity, 0.f, 4.f);
	}
	{
		ClassInfo& _info = Describe("SpotLightComponent", "Component");	// a cone of light, along the rotation of the actor
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<SpotLightComponent>(); };
		_info.AddProperty<SpotLightComponent, Color>("color", &SpotLightComponent::color);
		_info.AddProperty<SpotLightComponent, float>("intensity", &SpotLightComponent::intensity, 0.f, 4.f);
		_info.AddProperty<SpotLightComponent, float>("radius", &SpotLightComponent::radius, 1.f, 4000.f);
		_info.AddProperty<SpotLightComponent, float>("angle", &SpotLightComponent::angle, 1.f, 360.f);
		_info.AddProperty<SpotLightComponent, float>("softness", &SpotLightComponent::softness, 0.f, 1.f);
		_info.AddProperty<SpotLightComponent, bool>("castShadows", &SpotLightComponent::castShadows);
		AddDebugProperty<SpotLightComponent>(_info);	// the outline of the cone
	}
	{
		ClassInfo& _info = Describe("GameplayTagComponent", "Component");	// the gameplay tags of the actor
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<GameplayTagComponent>(); };
		_info.AddAccessor<GameplayTagComponent, GameplayTagContainer>("tags",
			[](GameplayTagComponent& _c) { return _c.GetTags(); },
			[](GameplayTagComponent& _c, const GameplayTagContainer& _v) { _c.SetTags(_v); });
		_info.AddProperty<GameplayTagComponent, EventList>("onTagAdded", &GameplayTagComponent::onTagAddedCalls);
		_info.AddProperty<GameplayTagComponent, EventList>("onTagRemoved", &GameplayTagComponent::onTagRemovedCalls);
		_info.AddFunction<GameplayTagComponent, GameplayTag>("AddTag", [](GameplayTagComponent& _c, const GameplayTag& _tag) { _c.AddTag(_tag); });
		_info.SetLastFunctionArgumentType(PropertyType::GameplayTag);
		_info.AddFunction<GameplayTagComponent, GameplayTag>("RemoveTag", [](GameplayTagComponent& _c, const GameplayTag& _tag) { _c.RemoveTag(_tag); });
		_info.SetLastFunctionArgumentType(PropertyType::GameplayTag);
		_info.AddFunction<GameplayTagComponent>("ClearTags", [](GameplayTagComponent& _c) { _c.ClearTags(); });
	}
	Describe("ShadowCasterComponent", "Component").componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<ShadowCasterComponent>(); };
	Describe("Light", "Actor").factory = [](Level* _level) -> Actor*
	{
		EmptyActor* _actor = _level->SpawnActor<EmptyActor>();
		_actor->SetName("Light");
		_actor->AddComponent<PointLightComponent>();
		_actor->AddComponent<BillboardComponent>(BillboardIcon::Light);
		return _actor;
	};
	Describe("SpotLight", "Actor").factory = [](Level* _level) -> Actor*
	{
		EmptyActor* _actor = _level->SpawnActor<EmptyActor>();
		_actor->SetName("SpotLight");
		_actor->AddComponent<SpotLightComponent>();
		_actor->AddComponent<BillboardComponent>(BillboardIcon::Light);
		return _actor;
	};
	Describe("GlobalLight", "Actor").factory = [](Level* _level) -> Actor*
	{
		EmptyActor* _actor = _level->SpawnActor<EmptyActor>();
		_actor->SetName("GlobalLight");
		_actor->AddComponent<GlobalLightComponent>();
		_actor->AddComponent<BillboardComponent>(BillboardIcon::Light);
		return _actor;
	};

	// ---- Navigation
	{
		ClassInfo& _info = Describe("NavMeshBoundsComponent", "Component");	// where a navigation mesh is built
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<NavMeshBoundsComponent>(); };
		_info.AddProperty<NavMeshBoundsComponent, FVector2>("size", &NavMeshBoundsComponent::size);
		_info.AddProperty<NavMeshBoundsComponent, float>("cellSize", &NavMeshBoundsComponent::cellSize, 2.f, 128.f);
		_info.AddProperty<NavMeshBoundsComponent, float>("agentRadius", &NavMeshBoundsComponent::agentRadius, 0.f, 512.f);
		_info.AddProperty<NavMeshBoundsComponent, int>("tileSize", &NavMeshBoundsComponent::tileSize, 8.f, 128.f);
		_info.AddProperty<NavMeshBoundsComponent, bool>("erodeEdges", &NavMeshBoundsComponent::erodeEdges);
		_info.AddFunction<NavMeshBoundsComponent>("Rebuild", [](NavMeshBoundsComponent& _c) { _c.Rebuild(); });
	}
	{
		ClassInfo& _info = Describe("NavModifierComponent", "Component");	// blocks the navigation, or makes it cost more or less
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<NavModifierComponent>(); };
		_info.AddProperty<NavModifierComponent, int>("area", &NavModifierComponent::area, 0.f, 1.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Blocked", "Cost" });
		_info.AddProperty<NavModifierComponent, float>("cost", &NavModifierComponent::cost, 0.05f, 100.f);
		_info.SetLastPropertyVisible<NavModifierComponent>([](NavModifierComponent& _c) { return _c.area == static_cast<int>(NavArea::Cost); });
		_info.AddProperty<NavModifierComponent, bool>("useOwnerShape", &NavModifierComponent::useOwnerShape);
		_info.AddProperty<NavModifierComponent, int>("shape", &NavModifierComponent::shape, 0.f, 1.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Box", "Circle" });
		_info.SetLastPropertyVisible<NavModifierComponent>([](NavModifierComponent& _c) { return !_c.useOwnerShape; });
		_info.AddProperty<NavModifierComponent, FVector2>("size", &NavModifierComponent::size);
		_info.SetLastPropertyVisible<NavModifierComponent>([](NavModifierComponent& _c) { return !_c.useOwnerShape && _c.shape == 0; });
		_info.AddProperty<NavModifierComponent, float>("radius", &NavModifierComponent::radius, 1.f, 5000.f);
		_info.SetLastPropertyVisible<NavModifierComponent>([](NavModifierComponent& _c) { return !_c.useOwnerShape && _c.shape == 1; });
	}
	{
		ClassInfo& _info = Describe("NavLinkComponent", "Component");	// a jump, a ladder, a teleporter
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<NavLinkComponent>(); };
		_info.AddProperty<NavLinkComponent, FVector2>("startOffset", &NavLinkComponent::startOffset);
		_info.AddProperty<NavLinkComponent, FVector2>("endOffset", &NavLinkComponent::endOffset);
		_info.AddProperty<NavLinkComponent, bool>("bidirectional", &NavLinkComponent::bidirectional);
		_info.AddProperty<NavLinkComponent, float>("cost", &NavLinkComponent::cost, 0.05f, 100.f);
	}
	{
		ClassInfo& _info = Describe("NavAgentComponent", "Component");	// moves its actor along paths
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<NavAgentComponent>(); };
		_info.AddProperty<NavAgentComponent, float>("speed", &NavAgentComponent::speed, 0.f, 5000.f);
		_info.AddProperty<NavAgentComponent, float>("acceleration", &NavAgentComponent::acceleration, 0.f, 50000.f);
		_info.AddProperty<NavAgentComponent, float>("stoppingDistance", &NavAgentComponent::stoppingDistance, 0.f, 1000.f);
		_info.AddProperty<NavAgentComponent, float>("radius", &NavAgentComponent::radius, 0.f, 512.f);
		_info.AddProperty<NavAgentComponent, bool>("rotateToMovement", &NavAgentComponent::rotateToMovement);
		_info.AddProperty<NavAgentComponent, float>("rotationSpeed", &NavAgentComponent::rotationSpeed, 0.f, 5000.f);
		_info.SetLastPropertyVisible<NavAgentComponent>([](NavAgentComponent& _c) { return _c.rotateToMovement; });
		_info.AddProperty<NavAgentComponent, bool>("avoidance", &NavAgentComponent::avoidance);
		_info.AddProperty<NavAgentComponent, float>("repathInterval", &NavAgentComponent::repathInterval, 0.05f, 10.f);
		_info.AddProperty<NavAgentComponent, int>("linkMode", &NavAgentComponent::linkMode, 0.f, 1.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Move along", "Teleport" });
		_info.AddProperty<NavAgentComponent, EventList>("onArrived", &NavAgentComponent::onArrivedCalls);
		_info.AddProperty<NavAgentComponent, EventList>("onFailed", &NavAgentComponent::onFailedCalls);
		_info.AddFunction<NavAgentComponent, FVector2>("MoveTo", [](NavAgentComponent& _c, const FVector2& _goal) { _c.MoveTo(_goal); });
		_info.AddFunction<NavAgentComponent, std::string>("MoveToActor", [](NavAgentComponent& _c, const std::string& _name)
		{
			for (const std::unique_ptr<Actor>& _actor : _c.GetLevel()->GetActorManager().GetActors())
				if (_actor->GetName() == _name && !_actor->IsPendingDestroy()) { _c.MoveToActor(_actor.get()); return; }
			LOG_WARNING("NavAgentComponent: MoveToActor: no actor named \"" + _name + "\"");
		});
		_info.AddFunction<NavAgentComponent, float>("Wander", [](NavAgentComponent& _c, const float& _radius) { _c.Wander(_radius); });
		_info.AddFunction<NavAgentComponent>("Stop", [](NavAgentComponent& _c) { _c.Stop(); });
	}
	{
		ClassInfo& _info = Describe("NavTestComponent", "Component");	// shows a path in the editor
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<NavTestComponent>(); };
		_info.AddProperty<NavTestComponent, FVector2>("goalOffset", &NavTestComponent::goalOffset);
		_info.AddProperty<NavTestComponent, float>("agentRadius", &NavTestComponent::agentRadius, 0.f, 512.f);
		_info.AddAccessor<NavTestComponent, float>("pathLength", [](NavTestComponent& _c) { return _c.Compute().length; }, [](NavTestComponent&, const float&) {}, 0.f, 0.f, true);
		_info.AddAccessor<NavTestComponent, bool>("reachable", [](NavTestComponent& _c) { const NavPath _p = _c.Compute(); return _p.found && !_p.partial; }, [](NavTestComponent&, const bool&) {}, 0.f, 0.f, true);
	}
	// Ready-made actors for the Add Actor list (saved as EmptyActors with these components).
	Describe("NavMeshBoundsVolume", "Actor").factory = [](Level* _level) -> Actor*
	{
		EmptyActor* _actor = _level->SpawnActor<EmptyActor>();
		_actor->SetName("NavMeshBoundsVolume");
		_actor->AddComponent<NavMeshBoundsComponent>();
		_actor->AddComponent<BillboardComponent>(BillboardIcon::Logic);
		return _actor;
	};
	Describe("NavigationTestActor", "Actor").factory = [](Level* _level) -> Actor*
	{
		EmptyActor* _actor = _level->SpawnActor<EmptyActor>();
		_actor->SetName("NavigationTestActor");
		_actor->AddComponent<NavTestComponent>();
		_actor->AddComponent<BillboardComponent>(BillboardIcon::Spawn);
		return _actor;
	};

	{
		ClassInfo& _info = Describe("BillboardComponent", "Component");	// an icon to click in the editor, never shown in the game
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<BillboardComponent>(); };
		_info.AddProperty<BillboardComponent, int>("icon", &BillboardComponent::icon, 0.f, 7.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Actor", "Camera", "Sound", "Light", "Effect", "Spawn", "Logic", "Star" });
		_info.SetLastPropertyVisible<BillboardComponent>([](BillboardComponent& _c) { return _c.texture.empty(); });
		_info.AddProperty<BillboardComponent, std::string>("texture", &BillboardComponent::texture);
		_info.SetLastPropertyType(PropertyType::Texture, { "(icon)" });
		_info.AddProperty<BillboardComponent, float>("size", &BillboardComponent::size, 8.f, 256.f);	// pixels on the screen
		_info.AddProperty<BillboardComponent, Color>("tint", &BillboardComponent::tint);
	}
	}
}
