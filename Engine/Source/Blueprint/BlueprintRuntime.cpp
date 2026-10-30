#include "Blueprint.h"
#include "Actors/Actor.h"
#include "GameFramework/Level.h"
#include "Utilities/Debug/Logger.h"

using namespace Krampus;

namespace
{
	constexpr int	MAX_STEPS	= 200000;	// nodes run in one go: more is a loop that never ends
	constexpr int	MAX_DEPTH	= 128;		// chains started from chains (custom events, pulled values)

	bool IsAlive(Level* _level, const Actor* _actor)
	{
		if (!_level || !_actor) return false;
		for (const std::unique_ptr<Actor>& _candidate : _level->GetActorManager().GetActors())
			if (_candidate.get() == _actor) return !_candidate->IsPendingDestroy();
		return false;
	}
}

// ================= NodeDesc =================

int NodeDesc::InputIndex(const std::string& _name) const
{
	for (size_t _i = 0; _i < inputs.size(); ++_i) if (inputs[_i].name == _name) return static_cast<int>(_i);
	return -1;
}

int NodeDesc::OutputIndex(const std::string& _name) const
{
	for (size_t _i = 0; _i < outputs.size(); ++_i) if (outputs[_i].name == _name) return static_cast<int>(_i);
	return -1;
}

// ================= NodeRun =================

BpValue NodeRun::In(const std::string& _pin)
{
	const int _index = instance.nodes[node].desc->InputIndex(_pin);
	if (_index < 0) return false;
	return instance.Pull(node, _index);
}

float NodeRun::InFloat(const std::string& _pin)			{ return std::get<float>(BlueprintValues::Convert(In(_pin), PinType::Float)); }
int NodeRun::InInt(const std::string& _pin)				{ return std::get<int>(BlueprintValues::Convert(In(_pin), PinType::Int)); }
bool NodeRun::InBool(const std::string& _pin)			{ return std::get<bool>(BlueprintValues::Convert(In(_pin), PinType::Bool)); }
std::string NodeRun::InString(const std::string& _pin)	{ return std::get<std::string>(BlueprintValues::Convert(In(_pin), PinType::String)); }
FVector2 NodeRun::InVector(const std::string& _pin)		{ return std::get<FVector2>(BlueprintValues::Convert(In(_pin), PinType::Vector2)); }
Color NodeRun::InColor(const std::string& _pin)			{ return std::get<Color>(BlueprintValues::Convert(In(_pin), PinType::Color)); }

Actor* NodeRun::InActor(const std::string& _pin)
{
	const BlueprintInstance::Compiled& _node = instance.nodes[node];
	const int _index = _node.desc->InputIndex(_pin);
	if (_index < 0 || _node.inputs[_index].first < 0) return instance.self;	// no wire: the blueprint's own actor
	Actor* _actor = std::get<Actor*>(BlueprintValues::Convert(In(_pin), PinType::Actor));
	return IsAlive(GetLevel(), _actor) ? _actor : nullptr;
}

std::string NodeRun::Literal(const std::string& _pin) const
{
	const BlueprintInstance::Compiled& _node = instance.nodes[node];
	const auto _typed = _node.source->values.find(_pin);
	if (_typed != _node.source->values.end()) return _typed->second;
	const int _index = _node.desc->InputIndex(_pin);
	return _index >= 0 ? _node.desc->inputs[_index].value : std::string();
}

void NodeRun::Out(const std::string& _pin, const BpValue& _value)
{
	BlueprintInstance::Compiled& _node = instance.nodes[node];
	const int _index = _node.desc->OutputIndex(_pin);
	if (_index >= 0) _node.outputs[_index] = BlueprintValues::Convert(_value, _node.desc->outputs[_index].type);
}

void NodeRun::Then(const std::string& _pin)
{
	const int _index = instance.nodes[node].desc->OutputIndex(_pin);
	if (_index >= 0) next.push_back(_index);
}

void NodeRun::RunNow(const std::string& _pin)
{
	const int _index = instance.nodes[node].desc->OutputIndex(_pin);
	if (_index >= 0) instance.FollowOutput(node, _index);
}

void NodeRun::Wait(const float& _seconds, const std::string& _pin)
{
	const int _index = instance.nodes[node].desc->OutputIndex(_pin);
	if (_index < 0) return;
	for (const BlueprintInstance::Waiting& _waiting : instance.waiting)
		if (_waiting.node == node) return;	// already counting: like Unreal, a Delay is not started again
	instance.waiting.push_back({ std::max(0.f, _seconds), node, _index });
}

std::string NodeRun::EnteredBy() const
{
	const BlueprintInstance::Compiled& _node = instance.nodes[node];
	return entered >= 0 && entered < static_cast<int>(_node.desc->inputs.size()) ? _node.desc->inputs[entered].name : std::string();
}

float& NodeRun::State()		{ return instance.nodes[node].state; }
Actor* NodeRun::Self() const	{ return instance.self; }
Level* NodeRun::GetLevel() const	{ return instance.self ? instance.self->GetLevel() : nullptr; }
float NodeRun::DeltaSeconds() const	{ return instance.delta; }
float NodeRun::Time() const		{ return instance.time; }

// ================= BlueprintInstance =================

bool BlueprintInstance::Compile(const std::shared_ptr<const BlueprintAsset>& _asset, Actor* _self, const std::string& _name, std::vector<std::string>* _errors)
{
	asset = _asset;
	self = _self;
	name = _name;
	nodes.clear();
	indexOf.clear();
	variables.clear();
	variableIndex.clear();
	waiting.clear();
	time = 0.f;
	stopped = false;
	if (!asset) return false;

	const auto _problem = [&](const std::string& _text)
	{
		if (_errors) _errors->push_back(_text);
		else LOG_WARNING("Blueprint " + name + ": " + _text);
	};

	for (const BlueprintVariable& _variable : asset->variables)
	{
		variableIndex[_variable.name] = static_cast<int>(variables.size());
		variables.push_back(BlueprintValues::Parse(_variable.value, static_cast<PinType>(_variable.type)));
	}

	for (const BlueprintNode& _node : asset->nodes)
	{
		const NodeDesc* _desc = BlueprintNodes::Find(_node.type, asset.get());
		if (!_desc)
		{
			_problem("node " + std::to_string(_node.id) + " is \"" + _node.type + "\", which does not exist (anymore?)");
			continue;
		}
		Compiled _compiled;
		_compiled.id = _node.id;
		_compiled.desc = _desc;
		_compiled.source = &_node;
		for (const PinDesc& _pin : _desc->inputs)
		{
			const auto _typed = _node.values.find(_pin.name);
			_compiled.literals.push_back(_pin.type == PinType::Exec ? BpValue(false)
				: BlueprintValues::Parse(_typed != _node.values.end() ? _typed->second : _pin.value, _pin.type));
		}
		_compiled.inputs.assign(_desc->inputs.size(), { -1, -1 });
		_compiled.exec.resize(_desc->outputs.size());
		for (const PinDesc& _pin : _desc->outputs) _compiled.outputs.push_back(BlueprintValues::Default(_pin.type));
		indexOf[_node.id] = static_cast<int>(nodes.size());
		nodes.push_back(std::move(_compiled));
	}

	for (const BlueprintLink& _link : asset->links)
	{
		const auto _from = indexOf.find(_link.fromNode), _to = indexOf.find(_link.toNode);
		if (_from == indexOf.end() || _to == indexOf.end()) continue;	// a node that was reported above
		Compiled& _source = nodes[_from->second];
		Compiled& _target = nodes[_to->second];
		const int _output = _source.desc->OutputIndex(_link.fromPin), _input = _target.desc->InputIndex(_link.toPin);
		if (_output < 0 || _input < 0)
		{
			_problem("a wire goes to a pin that does not exist: " + _source.desc->title + "." + _link.fromPin + " -> " + _target.desc->title + "." + _link.toPin);
			continue;
		}
		const PinType _outType = _source.desc->outputs[_output].type, _inType = _target.desc->inputs[_input].type;
		if (!BlueprintValues::CanConnect(_outType, _inType))
		{
			_problem(std::string("a ") + BlueprintValues::TypeName(_outType) + " wire goes into a " + BlueprintValues::TypeName(_inType) + " pin (" + _target.desc->title + "." + _link.toPin + ")");
			continue;
		}
		if (_outType == PinType::Exec) _source.exec[_output].push_back({ _to->second, _input });
		else _target.inputs[_input] = { _from->second, _output };
	}
	return !nodes.empty();
}

bool BlueprintInstance::Budget()
{
	if (stopped) return false;
	if (++steps <= MAX_STEPS) return true;
	LOG_ERROR("Blueprint " + name + ": more than " + std::to_string(MAX_STEPS) + " nodes in one go (a loop that never ends?): the blueprint is stopped");
	stopped = true;
	waiting.clear();
	return false;
}

void BlueprintInstance::ExecuteFrom(const int& _node, const int& _entered)
{
	if (stopped) return;
	if (depth >= MAX_DEPTH)
	{
		LOG_ERROR("Blueprint " + name + ": chains started inside chains more than " + std::to_string(MAX_DEPTH) + " times (a custom event that calls itself?)");
		return;
	}
	++depth;

	// Depth first: what follows a node runs before the next output of the node before it (Sequence: Then 0 ends before Then 1).
	std::vector<std::pair<int, int>> _stack{ { _node, _entered } };
	while (!_stack.empty() && Budget())
	{
		const auto [_index, _pin] = _stack.back();
		_stack.pop_back();
		nodes[_index].lastRun = time;

		NodeRun _run(*this, _index, _pin);
		if (nodes[_index].desc->run) nodes[_index].desc->run(_run);
		else _run.Then();

		for (auto _output = _run.next.rbegin(); _output != _run.next.rend(); ++_output)
		{
			const auto& _targets = nodes[_index].exec[*_output];
			for (auto _target = _targets.rbegin(); _target != _targets.rend(); ++_target) _stack.push_back(*_target);
		}
	}
	--depth;
}

void BlueprintInstance::FollowOutput(const int& _node, const int& _output)
{
	const std::vector<std::pair<int, int>> _targets = nodes[_node].exec[_output];	// copy: nothing may change it, but be safe
	for (const auto& [_target, _input] : _targets) ExecuteFrom(_target, _input);
}

BpValue BlueprintInstance::Pull(const int& _node, const int& _input)
{
	const Compiled& _compiled = nodes[_node];
	const auto [_from, _output] = _compiled.inputs[_input];
	const PinType _type = _compiled.desc->inputs[_input].type;
	if (_from < 0) return _type == PinType::Actor ? BpValue(self) : _compiled.literals[_input];
	if (nodes[_from].desc->pure) Evaluate(_from);	// a pure node is computed each time it is asked (like Unreal)
	return BlueprintValues::Convert(nodes[_from].outputs[_output], _type);
}

void BlueprintInstance::Evaluate(const int& _node)
{
	if (depth >= MAX_DEPTH || !Budget())
	{
		if (depth >= MAX_DEPTH && !stopped) { LOG_ERROR("Blueprint " + name + ": values that need themselves (a wire loop between pure nodes)"); stopped = true; }
		return;
	}
	++depth;
	nodes[_node].lastRun = time;
	NodeRun _run(*this, _node, -1);
	if (nodes[_node].desc->run) nodes[_node].desc->run(_run);
	--depth;
}

void BlueprintInstance::FireEvent(const std::string& _type, const std::string& _key)
{
	if (stopped) return;
	for (int _i = 0; _i < static_cast<int>(nodes.size()); ++_i)
	{
		const Compiled& _node = nodes[_i];
		if (_node.desc->id != _type) continue;
		if (!_key.empty() && !_node.desc->inputs.empty())
		{
			NodeRun _probe(*this, _i, -1);
			if (_probe.Literal(_node.desc->inputs.front().name) != _key) continue;
		}
		if (depth == 0) steps = 0;
		ExecuteFrom(_i, -1);
	}
}

void BlueprintInstance::Tick(const float& _deltaTime)
{
	if (stopped || nodes.empty()) return;
	delta = _deltaTime;
	time += _deltaTime;

	// What waits (Delay): the ones that are done leave the list before their chain runs (it may start a Delay again).
	if (!waiting.empty())
	{
		std::vector<Waiting> _done;
		for (Waiting& _waiting : waiting) _waiting.remaining -= _deltaTime;
		for (const Waiting& _waiting : waiting) if (_waiting.remaining <= 0.f) _done.push_back(_waiting);
		std::erase_if(waiting, [](const Waiting& _waiting) { return _waiting.remaining <= 0.f; });
		for (const Waiting& _waiting : _done)
		{
			steps = 0;
			FollowOutput(_waiting.node, _waiting.output);
		}
	}

	FireEvent("Event.Tick");
}

std::vector<std::string> BlueprintInstance::InputEvents() const
{
	std::vector<std::string> _names;
	for (int _i = 0; _i < static_cast<int>(nodes.size()); ++_i)
		if (nodes[_i].desc->id == "Event.Input")
		{
			NodeRun _probe(const_cast<BlueprintInstance&>(*this), _i, -1);
			_names.push_back(_probe.Literal("Input"));
		}
	return _names;
}

std::vector<std::string> BlueprintInstance::InputsHeldChecks() const
{
	std::vector<std::string> _names;
	for (int _i = 0; _i < static_cast<int>(nodes.size()); ++_i)
		if (nodes[_i].desc->id == "Input.IsHeld")
		{
			NodeRun _probe(const_cast<BlueprintInstance&>(*this), _i, -1);
			_names.push_back(_probe.Literal("Input"));
		}
	return _names;
}

BpValue BlueprintInstance::GetVariable(const std::string& _name) const
{
	const auto _found = variableIndex.find(_name);
	return _found == variableIndex.end() ? BpValue(0.f) : variables[_found->second];
}

void BlueprintInstance::SetVariable(const std::string& _name, const BpValue& _value)
{
	const auto _found = variableIndex.find(_name);
	if (_found == variableIndex.end()) return;
	const BlueprintVariable* _variable = asset ? asset->FindVariable(_name) : nullptr;
	variables[_found->second] = _variable ? BlueprintValues::Convert(_value, static_cast<PinType>(_variable->type)) : _value;
}

float BlueprintInstance::SinceRun(const int& _id) const
{
	const auto _found = indexOf.find(_id);
	return _found == indexOf.end() ? 1e9f : time - nodes[_found->second].lastRun;
}
