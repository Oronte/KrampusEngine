#include "Settings.h"
#include "Core/AssetPack.h"
#include "Core/Engine.h"
#include "Reflection/Reflection.h"
#include "Managers/LevelManager.h"
#include "Managers/WorldPhysics.h"
#include "Managers/TimerManager.h"
#include "Components/PhysicsComponent.h"

using namespace Krampus;

namespace
{
	Level* CurrentLevel(Engine* _engine)
	{
		return _engine ? _engine->GetLevelManager()->Peek() : nullptr;
	}

	std::string Trim(const std::string& _text)
	{
		const size_t _begin = _text.find_first_not_of(" \t\r\n");
		if (_begin == std::string::npos) return "";
		const size_t _end = _text.find_last_not_of(" \t\r\n");
		return _text.substr(_begin, _end - _begin + 1);
	}

	std::string FloatText(const float& _value)
	{
		char _buffer[48];
		std::snprintf(_buffer, sizeof(_buffer), "%.6g", _value);
		return _buffer;
	}

	std::string ValueToText(const PropertyValue& _value)
	{
		return std::visit([](const auto& _v) -> std::string
		{
			using V = std::decay_t<decltype(_v)>;
			if constexpr (std::is_same_v<V, bool>)				return _v ? "true" : "false";
			else if constexpr (std::is_same_v<V, int>)			return std::to_string(_v);
			else if constexpr (std::is_same_v<V, float>)		return FloatText(_v);
			else if constexpr (std::is_same_v<V, std::string>)	return _v;
			else if constexpr (std::is_same_v<V, FVector2>)		return FloatText(_v.x) + ", " + FloatText(_v.y);
			else												return std::to_string(_v.r) + ", " + std::to_string(_v.g) + ", " + std::to_string(_v.b) + ", " + std::to_string(_v.a);
		}, _value);
	}

	std::vector<float> Numbers(const std::string& _text)
	{
		std::vector<float> _result;
		std::stringstream _stream(_text);
		std::string _part;
		while (std::getline(_stream, _part, ','))
			_result.push_back(static_cast<float>(std::atof(_part.c_str())));
		return _result;
	}

	std::optional<PropertyValue> TextToValue(const PropertyType& _type, const std::string& _text)
	{
		switch (_type)
		{
		case PropertyType::Bool:	return PropertyValue(_text == "true" || _text == "1");
		case PropertyType::Int:
		case PropertyType::Flags:
		case PropertyType::Enum:	return PropertyValue(std::atoi(_text.c_str()));
		case PropertyType::Float:	return PropertyValue(static_cast<float>(std::atof(_text.c_str())));
		case PropertyType::String:
		case PropertyType::Texture:
		case PropertyType::Font:
		case PropertyType::Scene:
		case PropertyType::Events:
		case PropertyType::Sound:	return PropertyValue(_text);
		case PropertyType::Vector2:
		{
			const std::vector<float> _n = Numbers(_text);
			if (_n.size() == 2) return PropertyValue(FVector2(_n[0], _n[1]));
			break;
		}
		case PropertyType::Color:
		{
			const std::vector<float> _n = Numbers(_text);
			if (_n.size() == 4) return PropertyValue(Color(static_cast<std::uint8_t>(_n[0]), static_cast<std::uint8_t>(_n[1]), static_cast<std::uint8_t>(_n[2]), static_cast<std::uint8_t>(_n[3])));
			break;
		}
		}
		return std::nullopt;
	}
}

// ---- Logger

void LoggerSettings::ResetToDefaults()
{
	Logger::verboseLogging = false;
	Logger::veryVerboseLogging = false;
	Logger::canPlaySound = true;
	Logger::canPlayMsgSound = false;
	Logger::canPlayWarningSound = true;
	Logger::canPlayErrorSound = true;
	Logger::canPlayFatalSound = true;
	Logger::canColorText = true;
}

// ---- Physics

PhysicsSettings::PhysicsSettings()
{
	// The defaults are the ones of box2d (and of WorldPhysics: y goes down).
	const b2WorldDef _default = b2DefaultWorldDef();
	gravity = FVector2(-_default.gravity.x, -_default.gravity.y) * PX_TO_M;
	sleeping = _default.enableSleep;
	continuous = _default.enableContinuous;
	maxLinearSpeed = _default.maximumLinearSpeed;
	restitutionThreshold = _default.restitutionThreshold;
	hitEventThreshold = _default.hitEventThreshold;
	contactHertz = _default.contactHertz;
	contactDampingRatio = _default.contactDampingRatio;
	maxContactPushSpeed = _default.maxContactPushSpeed;
}

void PhysicsSettings::ApplyTo(WorldPhysics& _physics) const
{
	_physics.SetGravity(gravity);
	_physics.SetSubStepsCount(std::max(1, subSteps));
	_physics.EnableSleeping(sleeping);
	_physics.EnableContinuousPhysics(continuous);
	_physics.EnableWarmStarting(warmStarting);
	_physics.SetMaximumLinearSpeed(maxLinearSpeed);
	_physics.SetRestitutionThreshold(restitutionThreshold);
	_physics.SetHitEventThreshold(hitEventThreshold);
	_physics.SetContactTuning(contactHertz, contactDampingRatio, maxContactPushSpeed);
}

void PhysicsSettings::Apply()
{
	Level* _level = CurrentLevel(engine);
	if (!_level) return;

	ApplyTo(_level->GetWorldPhysics());

	// A body at rest is asleep and ignores a new gravity or a new threshold until something touches it: wake them all, so that the
	// change shows in the level at once.
	for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
		if (PhysicsComponent* _body = _actor->GetComponent<PhysicsComponent>())
			_body->SetAwake(true);
}

void PhysicsSettings::ApplyToLevel(Level& _level)
{
	ApplyTo(_level.GetWorldPhysics());
}

void PhysicsSettings::ResetToDefaults()
{
	Engine* _engine = engine;
	*this = PhysicsSettings();
	engine = _engine;	// the assignment copied a null engine
}

// ---- Time

void TimeSettings::Apply()
{
	if (Level* _level = CurrentLevel(engine))
		ApplyToLevel(*_level);
}

void TimeSettings::ApplyToLevel(Level& _level)
{
	_level.GetTimerManager().SetTimerScale(timeScale);
	// The window limit is only touched when the cap really changes (the editor window keeps its vertical sync otherwise).
	if (_level.GetTimerManager().GetMaxFrameRate() != static_cast<unsigned int>(std::max(0, maxFrameRate)))
		_level.GetTimerManager().SetMaxFrameRate(static_cast<unsigned int>(std::max(0, maxFrameRate)));
}

void TimeSettings::ResetToDefaults()
{
	timeScale = 1.f;
	maxFrameRate = 60;
}

// ---- Window

void WindowSettings::Apply()
{
	if (engine) engine->GetWindowRef().SetVerticalSyncEnabled(verticalSync);
}

void WindowSettings::ResetToDefaults()
{
	verticalSync = true;
}

// ---- Audio

void AudioSettings::Apply()
{
	sf::Listener::setGlobalVolume(globalVolume);
}

void AudioSettings::ResetToDefaults()
{
	globalVolume = 100.f;
}

void ShipSettings::ResetToDefaults()
{
	Engine* _engine = engine;
	*this = ShipSettings();
	engine = _engine;
}

// ---- All of them

EngineSettings::EngineSettings()
	: sections { &logger, &physics, &time, &window, &audio, &ship }
{
}

std::filesystem::path EngineSettings::GetPath()
{
	return std::filesystem::path(std::string(CONTENT_PATH)) / "Settings" / "EngineSettings.cfg";
}

void EngineSettings::Attach(Engine* _engine)
{
	for (SettingsSection* _section : sections)
		_section->Attach(_engine);
}

void EngineSettings::LoadFile()
{
	if (LoadSections(GetPath(), sections))
		LOG(VerbosityType::Log, "Settings: loaded " + GetPath().generic_string());
	else
		LOG(VerbosityType::Log, "Settings: no file yet (" + GetPath().generic_string() + "), defaults used");
}

void EngineSettings::Load()
{
	LoadFile();
	Apply();
}

bool EngineSettings::Save() const
{
	return SaveSections(GetPath(), sections);
}

void EngineSettings::ResetAllToDefaults()
{
	for (SettingsSection* _section : sections)
		_section->ResetToDefaults();
	Apply();
	LOG_MSG("Settings: every section back to its defaults");
}

void EngineSettings::Apply()
{
	for (SettingsSection* _section : sections)
		_section->Apply();
}

void EngineSettings::ApplyToLevel(Level& _level)
{
	for (SettingsSection* _section : sections)
		_section->ApplyToLevel(_level);
}

bool EngineSettings::SaveSections(const std::filesystem::path& _path, const std::vector<SettingsSection*>& _sections)
{
	std::error_code _error;
	if (_path.has_parent_path()) std::filesystem::create_directories(_path.parent_path(), _error);

	std::ofstream _file(_path, std::ios::trunc);
	if (!_file)
	{
		LOG_ERROR("Settings: cannot write " + _path.generic_string());
		return false;
	}

	_file << "# Settings of KrampusEngine. Edited in the editor (Settings menu); safe to edit by hand.\n";
	for (SettingsSection* _section : _sections)
	{
		const ClassInfo* _class = ClassRegistry::Find(ClassNameOf(*_section));
		if (!_class) continue;

		_file << "\n[" << _class->name << "]\n";
		for (const PropertyInfo* _property : _class->GetProperties())
			_file << _property->name << " = " << ValueToText(_property->get(_section)) << "\n";
	}

	LOG(VerbosityType::Verbose, "Settings: saved " + _path.generic_string());
	return _file.good();
}

bool EngineSettings::LoadSections(const std::filesystem::path& _path, const std::vector<SettingsSection*>& _sections)
{
	std::unique_ptr<std::istream> _stream = AssetPack::OpenStream(_path);	// in the exe of a packed game, else on the disk
	if (!_stream) return false;
	std::istream& _file = *_stream;

	SettingsSection* _current = nullptr;
	const ClassInfo* _class = nullptr;

	std::string _line;
	while (std::getline(_file, _line))
	{
		_line = Trim(_line);
		if (_line.empty() || _line[0] == '#') continue;

		if (_line.front() == '[' && _line.back() == ']')
		{
			const std::string _name = _line.substr(1, _line.size() - 2);
			_current = nullptr;
			_class = ClassRegistry::Find(_name);
			for (SettingsSection* _section : _sections)
				if (ClassNameOf(*_section) == _name) _current = _section;
			continue;
		}

		const size_t _equal = _line.find('=');
		if (!_current || !_class || _equal == std::string::npos) continue;

		const std::string _key = Trim(_line.substr(0, _equal));
		const std::string _text = Trim(_line.substr(_equal + 1));
		for (const PropertyInfo* _property : _class->GetProperties())
		{
			if (_property->name != _key) continue;
			if (std::optional<PropertyValue> _value = TextToValue(_property->type, _text))
				_property->set(_current, *_value);
		}
	}
	return true;
}
