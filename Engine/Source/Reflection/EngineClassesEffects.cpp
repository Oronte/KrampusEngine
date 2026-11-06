#include "EngineClassHelpers.h"

using namespace Krampus;
using namespace Krampus::EngineClassHelpers;

namespace Krampus::EngineClassHelpers
{
	void RegisterEffectClasses()
	{

	{
		ClassInfo& _info = Describe("ParticleComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<ParticleComponent>(); };

		// The preset first: loading a scene applies it, then the saved values that were changed after it.
		_info.AddAccessor<ParticleComponent, int>("preset",
			[](ParticleComponent& _c) { return static_cast<int>(_c.GetPreset()); },
			[](ParticleComponent& _c, const int& _v) { if (_v != static_cast<int>(_c.GetPreset())) _c.ApplyPreset(static_cast<ParticlePreset>(std::clamp(_v, 0, 8))); }, 0.f, 8.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Custom", "Fire", "Smoke", "Sparks", "Explosion", "Snow", "Rain", "Magic", "Blood" });

		// Emission
		AddParticleProperty(_info, "playOnStart", &ParticleSettings::playOnStart);
		AddParticleProperty(_info, "looping", &ParticleSettings::looping);
		AddParticleProperty(_info, "duration", &ParticleSettings::duration, 0.01f, 60.f);		// seconds of one cycle
		AddParticleProperty(_info, "rate", &ParticleSettings::rate, 0.f, 5000.f);				// particles per second
		AddParticleProperty(_info, "burst", &ParticleSettings::burst, 0.f, 10000.f);			// at the start of each cycle
		AddParticleProperty(_info, "maxParticles", &ParticleSettings::maxParticles, 1.f, 50000.f);
		AddParticleProperty(_info, "worldSpace", &ParticleSettings::worldSpace);
		_info.AddProperty<ParticleComponent, bool>("previewInEditor", &ParticleComponent::previewInEditor);
		_info.AddProperty<ParticleComponent, bool>("destroyOwnerWhenDone", &ParticleComponent::destroyOwnerWhenDone);

		// Shape of the emitter
		AddParticleProperty(_info, "shape", &ParticleSettings::shape, 0.f, 3.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Point", "Circle", "Rectangle", "Line" });
		AddParticleProperty(_info, "shapeRadius", &ParticleSettings::shapeRadius, 0.f, 5000.f);
		_info.SetLastPropertyVisible<ParticleComponent>([](ParticleComponent& _c) { return ParticleShapeOf(_c) == ParticleShape::Circle; });
		AddParticleProperty(_info, "shapeSize", &ParticleSettings::shapeSize);
		_info.SetLastPropertyVisible<ParticleComponent>([](ParticleComponent& _c) { return ParticleShapeOf(_c) == ParticleShape::Rectangle || ParticleShapeOf(_c) == ParticleShape::Line; });
		AddParticleProperty(_info, "fromEdge", &ParticleSettings::fromEdge);
		_info.SetLastPropertyVisible<ParticleComponent>([](ParticleComponent& _c) { return ParticleShapeOf(_c) == ParticleShape::Circle || ParticleShapeOf(_c) == ParticleShape::Rectangle; });
		AddParticleProperty(_info, "radial", &ParticleSettings::radial);					// thrown away from the center

		// At birth
		AddParticleProperty(_info, "lifetimeMin", &ParticleSettings::lifetimeMin, 0.01f, 60.f);
		AddParticleProperty(_info, "lifetimeMax", &ParticleSettings::lifetimeMax, 0.01f, 60.f);
		AddParticleProperty(_info, "speedMin", &ParticleSettings::speedMin);
		AddParticleProperty(_info, "speedMax", &ParticleSettings::speedMax);
		AddParticleProperty(_info, "direction", &ParticleSettings::direction, -360.f, 360.f);	// degrees, -90 = up
		_info.SetLastPropertyVisible<ParticleComponent>([](ParticleComponent& _c) { return !_c.Settings().radial; });
		AddParticleProperty(_info, "spread", &ParticleSettings::spread, 0.f, 360.f);			// degrees of the cone
		_info.SetLastPropertyVisible<ParticleComponent>([](ParticleComponent& _c) { return !_c.Settings().radial; });
		AddParticleProperty(_info, "sizeStart", &ParticleSettings::sizeStart, 0.f, 2000.f);
		AddParticleProperty(_info, "sizeEnd", &ParticleSettings::sizeEnd, 0.f, 2000.f);
		AddParticleProperty(_info, "sizeRandom", &ParticleSettings::sizeRandom, 0.f, 1.f);
		AddParticleProperty(_info, "rotationMin", &ParticleSettings::rotationMin, -360.f, 360.f);
		AddParticleProperty(_info, "rotationMax", &ParticleSettings::rotationMax, -360.f, 360.f);
		AddParticleProperty(_info, "spinMin", &ParticleSettings::spinMin);						// degrees per second
		AddParticleProperty(_info, "spinMax", &ParticleSettings::spinMax);
		AddParticleProperty(_info, "colorStart", &ParticleSettings::colorStart);
		AddParticleProperty(_info, "colorEnd", &ParticleSettings::colorEnd);					// alpha 0: fades out

		// During their life
		AddParticleProperty(_info, "gravity", &ParticleSettings::gravity);
		AddParticleProperty(_info, "drag", &ParticleSettings::drag, 0.f, 20.f);
		AddParticleProperty(_info, "stretch", &ParticleSettings::stretch, 0.f, 1.f);			// streaks along the speed

		// Marks on the actors that have a StainComponent
		const auto _stains = [](ParticleComponent& _c) { return _c.Settings().stainMode != static_cast<int>(ParticleStainMode::None); };
		AddParticleProperty(_info, "stainMode", &ParticleSettings::stainMode, 0.f, 2.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "None", "On death", "On hit (stops there)" });
		AddParticleProperty(_info, "stainColor", &ParticleSettings::stainColor);
		_info.SetLastPropertyVisible<ParticleComponent>(_stains);
		AddParticleProperty(_info, "stainSize", &ParticleSettings::stainSize, 0.5f, 500.f);		// radius in pixels
		_info.SetLastPropertyVisible<ParticleComponent>(_stains);
		AddParticleProperty(_info, "stainStyle", &ParticleSettings::stainStyle, 0.f, 2.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Soft", "Splat", "Scorch" });
		_info.SetLastPropertyVisible<ParticleComponent>(_stains);
		AddParticleProperty(_info, "stainChance", &ParticleSettings::stainChance, 0.f, 1.f);
		_info.SetLastPropertyVisible<ParticleComponent>(_stains);

		// Look
		AddParticleProperty(_info, "texture", &ParticleSettings::texture);
		_info.SetLastPropertyType(PropertyType::Texture, { "(soft dot)" });					// the name of the empty choice
		AddParticleProperty(_info, "blend", &ParticleSettings::blend, 0.f, 1.f);
		_info.SetLastPropertyType(PropertyType::Enum, { "Alpha", "Add (glow)" });
		AddDrawableProperties<ParticleComponent>(_info);

		_info.AddFunction<ParticleComponent>("Play", [](ParticleComponent& _c) { _c.Play(); });
		_info.AddFunction<ParticleComponent>("Stop", [](ParticleComponent& _c) { _c.Stop(); });
		_info.AddFunction<ParticleComponent>("Restart", [](ParticleComponent& _c) { _c.Restart(); });
		_info.AddFunction<ParticleComponent, int>("Burst", [](ParticleComponent& _c, const int& _count) { _c.Burst(_count); });
	}

	{
		ClassInfo& _info = Describe("StainComponent", "Component");	// lets particles and code mark the actor's sprite
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<StainComponent>(); };
		_info.AddProperty<StainComponent, float>("resolution", &StainComponent::resolution, 0.05f, 4.f);
		_info.AddProperty<StainComponent, float>("fadeSpeed", &StainComponent::fadeSpeed, 0.f, 5.f);	// 0: marks stay
		_info.AddProperty<StainComponent, float>("strength", &StainComponent::strength, 0.f, 1.f);
		_info.AddFunction<StainComponent>("Clear", [](StainComponent& _c) { _c.Clear(); });
	}

	{
		ClassInfo& _info = Describe("AudioListenerComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<AudioListenerComponent>(); };
		_info.AddAccessor<AudioListenerComponent, bool>("topView",
			[](AudioListenerComponent& _c) { return static_cast<bool>(_c.IsTopView()); },
			[](AudioListenerComponent& _c, const bool& _v) { _c.SetTopView(_v); });
		_info.AddAccessor<AudioListenerComponent, bool>("listening",
			[](AudioListenerComponent& _c) { return static_cast<bool>(_c.AudioListenerComponent::IsActive()); },
			[](AudioListenerComponent& _c, const bool& _v) { _c.AudioListenerComponent::SetActive(_v); });

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
			[](SpatialSoundComponent& _c) { return _c.soundFile.empty() ? std::string() : _c.soundFile + (_c.soundType == AudioExtensionType::WAV ? ".wav" : ".mp3"); },
			[](SpatialSoundComponent& _c, const std::string& _v)
			{
				const bool _hasExtension = _v.size() > 4 && _v[_v.size() - 4] == '.';
				const bool _wav = _hasExtension && (_v.compare(_v.size() - 4, 4, ".wav") == 0 || _v.compare(_v.size() - 4, 4, ".WAV") == 0);
				_c.SetSoundFile(String(_hasExtension ? _v.substr(0, _v.size() - 4) : _v), _wav ? AudioExtensionType::WAV : AudioExtensionType::MP3);
			});
		_info.SetLastPropertyType(PropertyType::Sound);
		_info.AddAccessor<SpatialSoundComponent, float>("volume",
			[](SpatialSoundComponent& _c) { return _c.GetSound().GetVolume(); },
			[](SpatialSoundComponent& _c, const float& _v) { _c.GetSound().SetVolume(_v); }, 0.f, 100.f);
		_info.AddAccessor<SpatialSoundComponent, bool>("loop",
			[](SpatialSoundComponent& _c) { return _c.GetSound().IsLooping(); },
			[](SpatialSoundComponent& _c, const bool& _v) { _c.GetSound().SetLoop(_v); });
		_info.AddAccessor<SpatialSoundComponent, float>("pitch",
			[](SpatialSoundComponent& _c) { return _c.GetSound().GetPitch(); },
			[](SpatialSoundComponent& _c, const float& _v) { _c.GetSound().SetPitch(std::max(0.01f, _v)); });
		_info.AddAccessor<SpatialSoundComponent, float>("attenuation",
			[](SpatialSoundComponent& _c) { return _c.GetSound().GetAttenuation(); },
			[](SpatialSoundComponent& _c, const float& _v) { _c.GetSound().SetAttenuation(_v); });
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
		_info.AddAccessor<AnimationComponentSM, std::string>("graph",		// edited in the Animation Graph window
			[](AnimationComponentSM& _c) { return AnimationGraph::Encode(_c.graph); },
			[](AnimationComponentSM& _c, const std::string& _v) { _c.graph = AnimationGraph::Decode(_v); _c.ResetParameters(); });
		_info.SetLastPropertyVisible<AnimationComponentSM>([](AnimationComponentSM&) { return false; });
		_info.AddFunction<AnimationComponentSM, std::string>("SetTrigger", [](AnimationComponentSM& _c, const std::string& _name) { _c.SetTrigger(_name); });
		_info.AddFunction<AnimationComponentSM, std::string>("PlayAnimation", [](AnimationComponentSM& _c, const std::string& _name) { _c.PlayAnimation(String(_name)); });
		// "name=value" (jump=1, speed=2.5): one argument only for the event lists.
		_info.AddFunction<AnimationComponentSM, std::string>("SetParameter", [](AnimationComponentSM& _c, const std::string& _text)
		{
			const size_t _equal = _text.find('=');
			if (_equal == std::string::npos) { LOG_WARNING("SetParameter: write name=value, e.g. jump=1"); return; }
			const std::string _name = _text.substr(0, _equal);
			const float _value = std::strtof(_text.c_str() + _equal + 1, nullptr);
			const AnimationParameter* _parameter = _c.graph.FindParameter(_name);
			if (_parameter && _parameter->type == static_cast<int>(AnimationParameterType::Int)) _c.SetInt(_name, static_cast<int>(std::round(_value)));
			else if (_parameter && _parameter->type == static_cast<int>(AnimationParameterType::Bool)) _c.SetBool(_name, _value != 0.f);
			else if (_parameter && _parameter->type == static_cast<int>(AnimationParameterType::Trigger)) { if (_value != 0.f) _c.SetTrigger(_name); else _c.ResetTrigger(_name); }
			else _c.SetFloat(_name, _value);
		});
	}

	{
		ClassInfo& _info = Describe("InputComponent", "Component");
		_info.componentFactory = [](Actor* _owner) -> Component* { return _owner->AddComponent<InputComponent>(); };
		_info.AddAccessor<InputComponent, std::string>("bindings",
			[](InputComponent& _c) { return InputComponent::EncodeBindings(_c.bindings); },
			[](InputComponent& _c, const std::string& _v) { _c.bindings = InputComponent::DecodeBindings(_v); _c.ApplyBindings(); });
		_info.SetLastPropertyType(PropertyType::InputBindings);
	}

	}
}
