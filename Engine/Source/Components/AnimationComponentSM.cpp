#include "AnimationComponentSM.h"
#include "Managers/TextureManager.h"
#include "SpriteComponent.h"
#include "Actors/Actor.h"
#include <charconv>
#include <sstream>


Krampus::AnimationSM::AnimationSM(Engine* _engine, AnimationComponentSM* _component, const String& _name, ShapeObject* _shape, const AnimationData& _data)
	: Animation(_engine, _name, _shape, _data)
{
	component = _component;
}

Bool Krampus::AnimationSM::CreateLink(AnimationSM* _animation, std::function<Bool()> _check)
{
	if (links.contains(_animation))
	{
		LOG_WARNING("You already have a link to this animation !");
		return false;
	}

	links[_animation] = _check;
	return true;
}

Krampus::AnimationSM* Krampus::AnimationSM::GetNextAnimation()
{
	// One step per frame: following the links of the next animation too would never end when two links point at each other.
	for (const std::pair<AnimationSM* const, std::function<Bool()>>& _pair : links)
		if (_pair.second && _pair.second())
			return _pair.first;
	return nullptr;
}

// ================= Graph (text saved in scenes) =================
// One record per line: "P|name|type|value", "T|from|to|waitForEnd|parameter,test,value;parameter,test,value", "N|name|x|y".

namespace
{
	std::string Clean(std::string _text)
	{
		std::replace_if(_text.begin(), _text.end(), [](const char _c) { return _c == '|' || _c == ';' || _c == ',' || _c == '\n' || _c == '\r'; }, '_');
		return _text;
	}

	std::vector<std::string> Split(const std::string& _text, const char _separator)
	{
		std::vector<std::string> _parts;
		size_t _start = 0;
		while (true)
		{
			const size_t _end = _text.find(_separator, _start);
			_parts.push_back(_text.substr(_start, _end == std::string::npos ? std::string::npos : _end - _start));
			if (_end == std::string::npos) break;
			_start = _end + 1;
		}
		return _parts;
	}

	float ToFloat(const std::string& _text)
	{
		float _value = 0.f;
		std::from_chars(_text.data(), _text.data() + _text.size(), _value);
		return _value;
	}

	int ToInt(const std::string& _text)
	{
		int _value = 0;
		std::from_chars(_text.data(), _text.data() + _text.size(), _value);
		return _value;
	}
}

std::string Krampus::AnimationGraph::Encode(const AnimationGraph& _graph)
{
	std::ostringstream _out;
	for (const AnimationParameter& _parameter : _graph.parameters)
		_out << "P|" << Clean(_parameter.name) << '|' << _parameter.type << '|' << _parameter.value << '\n';
	for (const AnimationTransition& _transition : _graph.transitions)
	{
		_out << "T|" << Clean(_transition.from) << '|' << Clean(_transition.to) << '|' << (_transition.waitForEnd ? 1 : 0) << '|';
		for (size_t _i = 0; _i < _transition.conditions.size(); ++_i)
		{
			const AnimationCondition& _condition = _transition.conditions[_i];
			if (_i > 0) _out << ';';
			_out << Clean(_condition.parameter) << ',' << _condition.test << ',' << _condition.value;
		}
		_out << '\n';
	}
	for (const auto& [_name, _position] : _graph.positions)
		_out << "N|" << Clean(_name) << '|' << _position.x << '|' << _position.y << '\n';
	return _out.str();
}

Krampus::AnimationGraph Krampus::AnimationGraph::Decode(const std::string& _text)
{
	AnimationGraph _graph;
	for (const std::string& _line : Split(_text, '\n'))
	{
		const std::vector<std::string> _values = Split(_line, '|');
		if (_values.empty()) continue;
		if (_values[0] == "P" && _values.size() >= 4)
			_graph.parameters.push_back({ _values[1], std::clamp(ToInt(_values[2]), 0, 3), ToFloat(_values[3]) });
		else if (_values[0] == "T" && _values.size() >= 5)
		{
			AnimationTransition _transition;
			_transition.from = _values[1];
			_transition.to = _values[2];
			_transition.waitForEnd = ToInt(_values[3]) != 0;
			if (!_values[4].empty())
				for (const std::string& _part : Split(_values[4], ';'))
				{
					const std::vector<std::string> _fields = Split(_part, ',');
					if (_fields.size() >= 3) _transition.conditions.push_back({ _fields[0], std::clamp(ToInt(_fields[1]), 0, 5), ToFloat(_fields[2]) });
				}
			_graph.transitions.push_back(std::move(_transition));
		}
		else if (_values[0] == "N" && _values.size() >= 4)
			_graph.positions[_values[1]] = FVector2(ToFloat(_values[2]), ToFloat(_values[3]));
	}
	return _graph;
}

const Krampus::AnimationParameter* Krampus::AnimationGraph::FindParameter(const std::string& _name) const
{
	for (const AnimationParameter& _parameter : parameters)
		if (_parameter.name == _name) return &_parameter;
	return nullptr;
}

// ================= Component =================

Krampus::AnimationComponentSM::AnimationComponentSM(Actor* _owner)
	: Component(_owner)
{
	name = NAME_OF(AnimationComponentSM);
}

Krampus::AnimationSM* Krampus::AnimationComponentSM::AddAnimation(const String& _name, const AnimationData& _data)
{
	if (animations.contains(_name))
	{
		LOG_ERROR("Can't add a animation with the same name than a other");
		return nullptr;
	}

	SpriteComponent* _sprite = owner->GetComponent<SpriteComponent>();
	if (!_sprite)
	{
		LOG_ERROR("AnimationComponentSM: \"" + _name + "\" needs a SpriteComponent on the same actor");
		return nullptr;
	}

	animations[_name] = std::make_unique<AnimationSM>(GetWorld(), this, _name, _sprite->GetShapeObject(), _data);

	AnimationSM* _anim = animations[_name].get();
	if (!current)
	{
		current = _anim;
		current->Update();
	}
	return _anim;
}

Krampus::AnimationSM* Krampus::AnimationComponentSM::AddAnimation(const String& _name, const AnimationData& _data, std::vector<std::pair<AnimationSM*, std::function<Bool()>>> _links)
{
	AnimationSM* _anim = AddAnimation(_name, _data);
	if (!_anim) return nullptr;

	for (const std::pair<AnimationSM*, std::function<Bool()>>& _pair : _links)
		_anim->CreateLink(_pair.first, _pair.second);

	return _anim;
}

void Krampus::AnimationComponentSM::BeginPlay()
{
	Component::BeginPlay();
	ResetParameters();
}

void Krampus::AnimationComponentSM::ResetParameters()
{
	values.clear();
	for (const AnimationParameter& _parameter : graph.parameters)
		values[_parameter.name] = _parameter.value;
}

void Krampus::AnimationComponentSM::SetValue(const std::string& _name, const float& _value, const AnimationParameterType& _type)
{
	if (!graph.FindParameter(_name) && !values.contains(_name))
		LOG_WARNING("AnimationComponentSM: \"" + owner->GetName().StdString() + "\" has no parameter \"" + _name + "\" in its graph: added for this game");
	values[_name] = _type == AnimationParameterType::Int ? std::round(_value) : _value;
}

void Krampus::AnimationComponentSM::SetBool(const std::string& _name, const bool& _value)	{ SetValue(_name, _value ? 1.f : 0.f, AnimationParameterType::Bool); }
void Krampus::AnimationComponentSM::SetInt(const std::string& _name, const int& _value)		{ SetValue(_name, static_cast<float>(_value), AnimationParameterType::Int); }
void Krampus::AnimationComponentSM::SetFloat(const std::string& _name, const float& _value)	{ SetValue(_name, _value, AnimationParameterType::Float); }
void Krampus::AnimationComponentSM::SetTrigger(const std::string& _name)					{ SetValue(_name, 1.f, AnimationParameterType::Trigger); }
void Krampus::AnimationComponentSM::ResetTrigger(const std::string& _name)					{ SetValue(_name, 0.f, AnimationParameterType::Trigger); }

float Krampus::AnimationComponentSM::GetParameter(const std::string& _name) const
{
	const auto _found = values.find(_name);
	if (_found != values.end()) return _found->second;
	const AnimationParameter* _parameter = graph.FindParameter(_name);
	return _parameter ? _parameter->value : 0.f;
}

bool Krampus::AnimationComponentSM::Passes(const AnimationTransition& _transition) const
{
	if (_transition.waitForEnd && current && current->GetCompletedCycles() == 0) return false;
	for (const AnimationCondition& _condition : _transition.conditions)
	{
		const float _value = GetParameter(_condition.parameter);
		bool _ok = false;
		switch (static_cast<AnimationTest>(_condition.test))
		{
		case AnimationTest::IsTrue:		_ok = _value != 0.f; break;
		case AnimationTest::IsFalse:	_ok = _value == 0.f; break;
		case AnimationTest::Equal:		_ok = std::abs(_value - _condition.value) < 0.0001f; break;
		case AnimationTest::NotEqual:	_ok = std::abs(_value - _condition.value) >= 0.0001f; break;
		case AnimationTest::Greater:	_ok = _value > _condition.value; break;
		case AnimationTest::Less:		_ok = _value < _condition.value; break;
		}
		if (!_ok) return false;
	}
	return true;
}

void Krampus::AnimationComponentSM::TakeGraphTransition()
{
	if (!current || graph.transitions.empty()) return;
	const std::string _currentName = current->GetName().StdString();

	// The ones from any animation first (a "hurt" or "die" wins over the rest), then the ones of the current animation, in their order.
	for (int _pass = 0; _pass < 2; ++_pass)
		for (const AnimationTransition& _transition : graph.transitions)
		{
			const bool _any = _transition.from == AnimationTransition::ANY;
			if ((_pass == 0) != _any) continue;
			if (!_any && _transition.from != _currentName) continue;
			if (_transition.to == _currentName) continue;	// already there (an Any transition would restart it every frame)
			if (!Passes(_transition)) continue;

			const auto _target = animations.find(String(_transition.to));
			if (_target == animations.end()) continue;

			// The triggers it tested are used up.
			for (const AnimationCondition& _condition : _transition.conditions)
				if (const AnimationParameter* _parameter = graph.FindParameter(_condition.parameter))
					if (_parameter->type == static_cast<int>(AnimationParameterType::Trigger)) values[_condition.parameter] = 0.f;

			LOG(VerbosityType::Verbose, "AnimationComponentSM: " + _currentName + " -> " + _transition.to);
			SwitchTo(_target->second.get());
			return;
		}
}

void Krampus::AnimationComponentSM::SwitchTo(AnimationSM* _next)
{
	if (!_next || _next == current) return;
	if (current) current->Stop();
	current = _next;
	current->Start();
}

void Krampus::AnimationComponentSM::PlayAnimation(const String& _name)
{
	const auto _found = animations.find(_name);
	if (_found == animations.end())
	{
		LOG_WARNING("AnimationComponentSM: no animation \"" + _name.StdString() + "\" on \"" + owner->GetName().StdString() + "\"");
		return;
	}
	if (_found->second.get() == current) { current->Start(); return; }
	SwitchTo(_found->second.get());
}

void Krampus::AnimationComponentSM::Tick(const Float& _deltaTime)
{
	Component::Tick(_deltaTime);

	if (!clipStarted)
	{
		clipStarted = true;
		if (animations.empty()) ApplyClips();
		if (playOnBeginPlay && current) current->Start();
	}

	if (!current) return;
	AnimationSM* const _before = current;
	TakeGraphTransition();
	if (current != _before) return;	// one change per frame

	SwitchTo(current->GetNextAnimation());	// links made in code
}

std::string Krampus::AnimationComponentSM::ToString() const
{
	String _animationsName;
	for (auto& [_key, _anim] : animations)
		_animationsName += _key + ", ";
	return name + std::format(" -> Animations Count = {} : ", animations.size()) + _animationsName;
}

void Krampus::AnimationComponentSM::ApplyClips()
{
	if (!owner->GetComponent<SpriteComponent>()) return;

	// The animations of the old clips are deleted: no code link may still point at them.
	for (const String& _old : appliedClips)
	{
		const auto _found = animations.find(_old);
		if (_found == animations.end()) continue;
		for (auto& [_name, _animation] : animations)
			_animation->RemoveLinksTo(_found->second.get());
	}

	ApplyAnimationClips(clips, startAnimation, appliedClips, animations, current, [this](const String& _name, const AnimationData& _data) { AddAnimation(_name, _data); }, "AnimationComponentSM");
	clipStarted = false;
}
