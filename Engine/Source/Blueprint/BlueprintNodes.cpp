#include "Blueprint.h"
#include "Actors/Actor.h"
#include "Components/ParticleComponent.h"
#include "Components/StainComponent.h"
#include "GameFramework/Level.h"
#include "Reflection/Reflection.h"
#include "Utilities/Debug/Logger.h"
#include <cmath>
#include <optional>
#include <random>

using namespace Krampus;

namespace
{
	constexpr float DEG = 3.14159265f / 180.f;

	// ---- Building the library

	PinDesc ExecPin(const std::string& _name = "In")				{ return { _name, PinType::Exec, "", {} }; }
	PinDesc Pin(const std::string& _name, const PinType& _type, const std::string& _value = "") { return { _name, _type, _value, {} }; }
	PinDesc EnumPin(const std::string& _name, const std::string& _value, std::vector<std::string> _names) { return { _name, PinType::Int, _value, std::move(_names) }; }

	std::vector<std::unique_ptr<NodeDesc>>& Library()
	{
		static std::vector<std::unique_ptr<NodeDesc>> _library;
		return _library;
	}

	std::unordered_map<std::string, std::unique_ptr<NodeDesc>>& Generated()
	{
		static std::unordered_map<std::string, std::unique_ptr<NodeDesc>> _generated;
		return _generated;
	}

	// An action node: In -> ... -> Then.
	void Action(const std::string& _id, const std::string& _title, const std::string& _category, const std::string& _tooltip,
		std::vector<PinDesc> _inputs, std::vector<PinDesc> _outputs, std::function<void(NodeRun&)> _run)
	{
		auto _node = std::make_unique<NodeDesc>();
		_node->id = _id; _node->title = _title; _node->category = _category; _node->tooltip = _tooltip;
		_node->inputs.push_back(ExecPin());
		for (PinDesc& _pin : _inputs) _node->inputs.push_back(std::move(_pin));
		_node->outputs.push_back(ExecPin("Then"));
		for (PinDesc& _pin : _outputs) _node->outputs.push_back(std::move(_pin));
		_node->run = [_run](NodeRun& _r) { _run(_r); _r.Then(); };
		Library().push_back(std::move(_node));
	}

	// A node with exec pins of its own (flow, events).
	void Custom(const std::string& _id, const std::string& _title, const std::string& _category, const std::string& _tooltip,
		std::vector<PinDesc> _inputs, std::vector<PinDesc> _outputs, std::function<void(NodeRun&)> _run, const bool _event = false)
	{
		auto _node = std::make_unique<NodeDesc>();
		_node->id = _id; _node->title = _title; _node->category = _category; _node->tooltip = _tooltip;
		_node->inputs = std::move(_inputs); _node->outputs = std::move(_outputs);
		_node->run = std::move(_run);
		_node->event = _event;
		Library().push_back(std::move(_node));
	}

	// A value node: no exec pin, computed when pulled.
	void Pure(const std::string& _id, const std::string& _title, const std::string& _category, const std::string& _tooltip,
		std::vector<PinDesc> _inputs, std::vector<PinDesc> _outputs, std::function<void(NodeRun&)> _run)
	{
		auto _node = std::make_unique<NodeDesc>();
		_node->id = _id; _node->title = _title; _node->category = _category; _node->tooltip = _tooltip;
		_node->inputs = std::move(_inputs); _node->outputs = std::move(_outputs);
		_node->run = std::move(_run);
		_node->pure = true;
		Library().push_back(std::move(_node));
	}

	void FloatMath(const std::string& _id, const std::string& _title, const std::string& _b, std::function<float(float, float)> _op)
	{
		Pure("Math." + _id, _title, "Math", _title + " of two numbers", { Pin("A", PinType::Float, "0"), Pin("B", PinType::Float, _b) },
			{ Pin("Result", PinType::Float) }, [_op](NodeRun& _r) { _r.Out("Result", _op(_r.InFloat("A"), _r.InFloat("B"))); });
	}

	void Compare(const std::string& _id, const std::string& _title, std::function<bool(float, float)> _op)
	{
		Pure("Math." + _id, _title, "Math", "Compares two numbers", { Pin("A", PinType::Float, "0"), Pin("B", PinType::Float, "0") },
			{ Pin("Result", PinType::Bool) }, [_op](NodeRun& _r) { _r.Out("Result", _op(_r.InFloat("A"), _r.InFloat("B"))); });
	}

	float Random(const float _min, const float _max)
	{
		static std::mt19937 _engine{ std::random_device{}() };
		return _min + (_max - _min) * std::uniform_real_distribution<float>(0.f, 1.f)(_engine);
	}

	void BuildLibrary()
	{
		static bool _built = false;
		if (_built) return;
		_built = true;
		const std::vector<std::string> PRESETS = { "Custom", "Fire", "Smoke", "Sparks", "Explosion", "Snow", "Rain", "Magic", "Blood" };

		// ---- Events
		Custom("Event.BeginPlay", "Event BeginPlay", "Events", "When the game starts (or the actor is spawned)", {}, { ExecPin("Then") },
			[](NodeRun& _r) { _r.Then(); }, true);
		Custom("Event.Tick", "Event Tick", "Events", "Every frame", {}, { ExecPin("Then"), Pin("Delta Seconds", PinType::Float) },
			[](NodeRun& _r) { _r.Out("Delta Seconds", _r.DeltaSeconds()); _r.Then(); }, true);
		Custom("Event.Input", "Event Input", "Events", "When an input happens: \"Space.Press\", \"A.Release\", \"MouseLeft.Press\"...",
			{ Pin("Input", PinType::String, "Space.Press") }, { ExecPin("Then") }, [](NodeRun& _r) { _r.Then(); }, true);
		Custom("Event.Custom", "Custom Event", "Events", "Started by a Call Event node, or by CallEvent in an event list (a button...)",
			{ Pin("Name", PinType::String, "MyEvent") }, { ExecPin("Then") }, [](NodeRun& _r) { _r.Then(); }, true);

		// ---- Flow
		Custom("Flow.Branch", "Branch", "Flow", "Goes one way or the other (if)", { ExecPin(), Pin("Condition", PinType::Bool, "true") },
			{ ExecPin("True"), ExecPin("False") }, [](NodeRun& _r) { _r.Then(_r.InBool("Condition") ? "True" : "False"); });
		Custom("Flow.Sequence", "Sequence", "Flow", "Runs its outputs one after the other", { ExecPin() },
			{ ExecPin("Then 0"), ExecPin("Then 1"), ExecPin("Then 2"), ExecPin("Then 3") },
			[](NodeRun& _r) { _r.Then("Then 0"); _r.Then("Then 1"); _r.Then("Then 2"); _r.Then("Then 3"); });
		Custom("Flow.DoOnce", "Do Once", "Flow", "Lets through the first time only (until Reset)", { ExecPin(), ExecPin("Reset") }, { ExecPin("Completed") },
			[](NodeRun& _r)
			{
				if (_r.EnteredBy() == "Reset") { _r.State() = 0.f; return; }
				if (_r.State() == 0.f) { _r.State() = 1.f; _r.Then("Completed"); }
			});
		Custom("Flow.FlipFlop", "Flip Flop", "Flow", "A, then B, then A...", { ExecPin() }, { ExecPin("A"), ExecPin("B"), Pin("Is A", PinType::Bool) },
			[](NodeRun& _r)
			{
				const bool _a = _r.State() == 0.f;
				_r.State() = _a ? 1.f : 0.f;
				_r.Out("Is A", _a);
				_r.Then(_a ? "A" : "B");
			});
		Custom("Flow.Gate", "Gate", "Flow", "Lets Enter through while open", { ExecPin("Enter"), ExecPin("Open"), ExecPin("Close"), ExecPin("Toggle"), Pin("Start Open", PinType::Bool, "true") },
			{ ExecPin("Exit") },
			[](NodeRun& _r)
			{
				float& _state = _r.State();	// 0: not set yet, 1: open, 2: closed
				if (_state == 0.f) _state = _r.InBool("Start Open") ? 1.f : 2.f;
				const std::string _by = _r.EnteredBy();
				if (_by == "Open") _state = 1.f;
				else if (_by == "Close") _state = 2.f;
				else if (_by == "Toggle") _state = _state == 1.f ? 2.f : 1.f;
				else if (_state == 1.f) _r.Then("Exit");
			});
		Custom("Flow.Delay", "Delay", "Flow", "Goes on after some seconds (started again while waiting: ignored)", { ExecPin(), Pin("Duration", PinType::Float, "1") },
			{ ExecPin("Completed") }, [](NodeRun& _r) { _r.Wait(_r.InFloat("Duration"), "Completed"); });
		Custom("Flow.ForLoop", "For Loop", "Flow", "Runs Loop Body for each Index from First to Last, then Completed",
			{ ExecPin(), Pin("First", PinType::Int, "0"), Pin("Last", PinType::Int, "9") }, { ExecPin("Loop Body"), Pin("Index", PinType::Int), ExecPin("Completed") },
			[](NodeRun& _r)
			{
				const int _last = _r.InInt("Last");
				for (int _i = _r.InInt("First"); _i <= _last; ++_i)
				{
					_r.Out("Index", _i);
					_r.RunNow("Loop Body");
				}
				_r.Then("Completed");
			});
		Action("Flow.CallEvent", "Call Event", "Flow", "Starts the Custom Events of this name", { Pin("Name", PinType::String, "MyEvent") }, {},
			[](NodeRun& _r) { _r.Instance().FireEvent("Event.Custom", _r.InString("Name")); });

		// ---- Math
		FloatMath("Add", "Add", "0", [](float _a, float _b) { return _a + _b; });
		FloatMath("Subtract", "Subtract", "0", [](float _a, float _b) { return _a - _b; });
		FloatMath("Multiply", "Multiply", "1", [](float _a, float _b) { return _a * _b; });
		FloatMath("Divide", "Divide", "1", [](float _a, float _b) { return _b == 0.f ? 0.f : _a / _b; });
		FloatMath("Modulo", "Modulo", "1", [](float _a, float _b) { return _b == 0.f ? 0.f : std::fmod(_a, _b); });
		FloatMath("Power", "Power", "2", [](float _a, float _b) { return std::pow(_a, _b); });
		FloatMath("Min", "Min", "0", [](float _a, float _b) { return std::min(_a, _b); });
		FloatMath("Max", "Max", "0", [](float _a, float _b) { return std::max(_a, _b); });
		Pure("Math.Clamp", "Clamp", "Math", "Keeps a number between Min and Max", { Pin("Value", PinType::Float), Pin("Min", PinType::Float, "0"), Pin("Max", PinType::Float, "1") },
			{ Pin("Result", PinType::Float) }, [](NodeRun& _r) { _r.Out("Result", std::clamp(_r.InFloat("Value"), _r.InFloat("Min"), std::max(_r.InFloat("Min"), _r.InFloat("Max")))); });
		Pure("Math.Lerp", "Lerp", "Math", "From A (Alpha 0) to B (Alpha 1)", { Pin("A", PinType::Float, "0"), Pin("B", PinType::Float, "1"), Pin("Alpha", PinType::Float, "0.5") },
			{ Pin("Result", PinType::Float) }, [](NodeRun& _r) { _r.Out("Result", _r.InFloat("A") + (_r.InFloat("B") - _r.InFloat("A")) * _r.InFloat("Alpha")); });
		Pure("Math.Abs", "Abs", "Math", "The number without its sign", { Pin("Value", PinType::Float) }, { Pin("Result", PinType::Float) },
			[](NodeRun& _r) { _r.Out("Result", std::abs(_r.InFloat("Value"))); });
		Pure("Math.Sqrt", "Square Root", "Math", "", { Pin("Value", PinType::Float, "1") }, { Pin("Result", PinType::Float) },
			[](NodeRun& _r) { _r.Out("Result", std::sqrt(std::max(0.f, _r.InFloat("Value")))); });
		Pure("Math.Sin", "Sin", "Math", "Of an angle in degrees", { Pin("Degrees", PinType::Float) }, { Pin("Result", PinType::Float) },
			[](NodeRun& _r) { _r.Out("Result", std::sin(_r.InFloat("Degrees") * DEG)); });
		Pure("Math.Cos", "Cos", "Math", "Of an angle in degrees", { Pin("Degrees", PinType::Float) }, { Pin("Result", PinType::Float) },
			[](NodeRun& _r) { _r.Out("Result", std::cos(_r.InFloat("Degrees") * DEG)); });
		Pure("Math.Round", "Round", "Math", "To the closest whole number", { Pin("Value", PinType::Float) }, { Pin("Result", PinType::Int) },
			[](NodeRun& _r) { _r.Out("Result", static_cast<int>(std::lround(_r.InFloat("Value")))); });
		Pure("Math.Floor", "Floor", "Math", "To the whole number below", { Pin("Value", PinType::Float) }, { Pin("Result", PinType::Int) },
			[](NodeRun& _r) { _r.Out("Result", static_cast<int>(std::floor(_r.InFloat("Value")))); });
		Pure("Math.RandomFloat", "Random Float", "Math", "A new random number each time it is pulled", { Pin("Min", PinType::Float, "0"), Pin("Max", PinType::Float, "1") },
			{ Pin("Result", PinType::Float) }, [](NodeRun& _r) { _r.Out("Result", Random(_r.InFloat("Min"), _r.InFloat("Max"))); });
		Pure("Math.RandomInt", "Random Int", "Math", "From Min to Max, both included", { Pin("Min", PinType::Int, "0"), Pin("Max", PinType::Int, "10") },
			{ Pin("Result", PinType::Int) }, [](NodeRun& _r)
			{
				const int _min = _r.InInt("Min"), _max = std::max(_min, _r.InInt("Max"));
				_r.Out("Result", std::min(_max, _min + static_cast<int>(Random(0.f, static_cast<float>(_max - _min + 1)))));
			});
		Pure("Math.AddInt", "Add (Int)", "Math", "", { Pin("A", PinType::Int, "0"), Pin("B", PinType::Int, "1") }, { Pin("Result", PinType::Int) },
			[](NodeRun& _r) { _r.Out("Result", _r.InInt("A") + _r.InInt("B")); });
		Pure("Math.SubtractInt", "Subtract (Int)", "Math", "", { Pin("A", PinType::Int, "0"), Pin("B", PinType::Int, "1") }, { Pin("Result", PinType::Int) },
			[](NodeRun& _r) { _r.Out("Result", _r.InInt("A") - _r.InInt("B")); });
		Compare("Greater", "A > B", [](float _a, float _b) { return _a > _b; });
		Compare("Less", "A < B", [](float _a, float _b) { return _a < _b; });
		Compare("GreaterEqual", "A >= B", [](float _a, float _b) { return _a >= _b; });
		Compare("LessEqual", "A <= B", [](float _a, float _b) { return _a <= _b; });
		Compare("Equal", "A == B", [](float _a, float _b) { return std::abs(_a - _b) < 0.0001f; });
		Compare("NotEqual", "A != B", [](float _a, float _b) { return std::abs(_a - _b) >= 0.0001f; });

		// ---- Logic
		Pure("Logic.And", "AND", "Logic", "True when both are", { Pin("A", PinType::Bool), Pin("B", PinType::Bool) }, { Pin("Result", PinType::Bool) },
			[](NodeRun& _r) { _r.Out("Result", _r.InBool("A") && _r.InBool("B")); });
		Pure("Logic.Or", "OR", "Logic", "True when one is", { Pin("A", PinType::Bool), Pin("B", PinType::Bool) }, { Pin("Result", PinType::Bool) },
			[](NodeRun& _r) { _r.Out("Result", _r.InBool("A") || _r.InBool("B")); });
		Pure("Logic.Not", "NOT", "Logic", "The opposite", { Pin("Value", PinType::Bool) }, { Pin("Result", PinType::Bool) },
			[](NodeRun& _r) { _r.Out("Result", !_r.InBool("Value")); });
		Pure("Logic.Xor", "XOR", "Logic", "True when only one is", { Pin("A", PinType::Bool), Pin("B", PinType::Bool) }, { Pin("Result", PinType::Bool) },
			[](NodeRun& _r) { _r.Out("Result", _r.InBool("A") != _r.InBool("B")); });

		// ---- Vectors
		Pure("Vector.Make", "Make Vector", "Vector", "", { Pin("X", PinType::Float), Pin("Y", PinType::Float) }, { Pin("Vector", PinType::Vector2) },
			[](NodeRun& _r) { _r.Out("Vector", FVector2(_r.InFloat("X"), _r.InFloat("Y"))); });
		Pure("Vector.Break", "Break Vector", "Vector", "", { Pin("Vector", PinType::Vector2) }, { Pin("X", PinType::Float), Pin("Y", PinType::Float) },
			[](NodeRun& _r) { const FVector2 _v = _r.InVector("Vector"); _r.Out("X", _v.x); _r.Out("Y", _v.y); });
		Pure("Vector.Add", "Add (Vector)", "Vector", "", { Pin("A", PinType::Vector2), Pin("B", PinType::Vector2) }, { Pin("Result", PinType::Vector2) },
			[](NodeRun& _r) { _r.Out("Result", _r.InVector("A") + _r.InVector("B")); });
		Pure("Vector.Subtract", "Subtract (Vector)", "Vector", "", { Pin("A", PinType::Vector2), Pin("B", PinType::Vector2) }, { Pin("Result", PinType::Vector2) },
			[](NodeRun& _r) { _r.Out("Result", _r.InVector("A") - _r.InVector("B")); });
		Pure("Vector.Scale", "Scale (Vector)", "Vector", "Each part times the factor", { Pin("Vector", PinType::Vector2), Pin("Factor", PinType::Float, "1") }, { Pin("Result", PinType::Vector2) },
			[](NodeRun& _r) { _r.Out("Result", _r.InVector("Vector") * _r.InFloat("Factor")); });
		Pure("Vector.Length", "Length", "Vector", "", { Pin("Vector", PinType::Vector2) }, { Pin("Length", PinType::Float) },
			[](NodeRun& _r) { _r.Out("Length", static_cast<float>(_r.InVector("Vector").Length())); });
		Pure("Vector.Normalize", "Normalize", "Vector", "Same way, length 1", { Pin("Vector", PinType::Vector2) }, { Pin("Result", PinType::Vector2) },
			[](NodeRun& _r)
			{
				const FVector2 _v = _r.InVector("Vector");
				const float _length = _v.Length();
				_r.Out("Result", _length > 0.0001f ? _v * (1.f / _length) : FVector2(0.f, 0.f));
			});
		Pure("Vector.Distance", "Distance", "Vector", "Between two points", { Pin("A", PinType::Vector2), Pin("B", PinType::Vector2) }, { Pin("Distance", PinType::Float) },
			[](NodeRun& _r) { _r.Out("Distance", static_cast<float>((_r.InVector("B") - _r.InVector("A")).Length())); });
		Pure("Vector.FromAngle", "Vector From Angle", "Vector", "Length 1 toward an angle (0 = right, 90 = down)", { Pin("Degrees", PinType::Float) }, { Pin("Vector", PinType::Vector2) },
			[](NodeRun& _r) { const float _a = _r.InFloat("Degrees") * DEG; _r.Out("Vector", FVector2(std::cos(_a), std::sin(_a))); });

		// ---- Text
		Pure("String.Append", "Append", "String", "A then B", { Pin("A", PinType::String), Pin("B", PinType::String) }, { Pin("Result", PinType::String) },
			[](NodeRun& _r) { _r.Out("Result", _r.InString("A") + _r.InString("B")); });
		Pure("String.Equal", "Equal (Text)", "String", "", { Pin("A", PinType::String), Pin("B", PinType::String) }, { Pin("Result", PinType::Bool) },
			[](NodeRun& _r) { _r.Out("Result", _r.InString("A") == _r.InString("B")); });

		// ---- Actors
		Pure("Actor.Self", "Self", "Actor", "The actor that runs this blueprint", {}, { Pin("Self", PinType::Actor) },
			[](NodeRun& _r) { _r.Out("Self", _r.Self()); });
		Pure("Actor.GetPosition", "Get Position", "Actor", "", { Pin("Target", PinType::Actor) }, { Pin("Position", PinType::Vector2) },
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _r.Out("Position", _a->GetPosition()); });
		Action("Actor.SetPosition", "Set Position", "Actor", "", { Pin("Target", PinType::Actor), Pin("Position", PinType::Vector2) }, {},
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _a->SetPosition(_r.InVector("Position")); });
		Action("Actor.AddOffset", "Add Offset", "Actor", "Moves by a vector", { Pin("Target", PinType::Actor), Pin("Offset", PinType::Vector2) }, {},
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _a->SetPosition(_a->GetPosition() + _r.InVector("Offset")); });
		Action("Actor.MoveTowards", "Move Towards", "Actor", "Moves toward a point at a speed (pixels per second): put it on Event Tick",
			{ Pin("Target", PinType::Actor), Pin("Goal", PinType::Vector2), Pin("Speed", PinType::Float, "200") }, { Pin("Arrived", PinType::Bool) },
			[](NodeRun& _r)
			{
				Actor* _a = _r.InActor("Target");
				if (!_a) return;
				const FVector2 _to = _r.InVector("Goal") - _a->GetPosition();
				const float _distance = _to.Length(), _step = _r.InFloat("Speed") * _r.DeltaSeconds();
				_a->SetPosition(_distance <= _step || _distance < 0.0001f ? _r.InVector("Goal") : _a->GetPosition() + _to * (_step / _distance));
				_r.Out("Arrived", _distance <= _step);
			});
		Pure("Actor.GetRotation", "Get Rotation", "Actor", "In degrees", { Pin("Target", PinType::Actor) }, { Pin("Degrees", PinType::Float) },
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _r.Out("Degrees", static_cast<float>(_a->GetRotation().ToDegrees())); });
		Action("Actor.SetRotation", "Set Rotation", "Actor", "In degrees", { Pin("Target", PinType::Actor), Pin("Degrees", PinType::Float) }, {},
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _a->SetRotation(Angle::FromDegrees(_r.InFloat("Degrees"))); });
		Action("Actor.AddRotation", "Add Rotation", "Actor", "Turns by some degrees", { Pin("Target", PinType::Actor), Pin("Degrees", PinType::Float, "90") }, {},
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _a->SetRotation(Angle::FromDegrees(_a->GetRotation().ToDegrees() + _r.InFloat("Degrees"))); });
		Pure("Actor.GetScale", "Get Scale", "Actor", "", { Pin("Target", PinType::Actor) }, { Pin("Scale", PinType::Vector2) },
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _r.Out("Scale", _a->GetScale()); });
		Action("Actor.SetScale", "Set Scale", "Actor", "", { Pin("Target", PinType::Actor), Pin("Scale", PinType::Vector2, "1, 1") }, {},
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _a->SetScale(_r.InVector("Scale")); });
		Action("Actor.SetActive", "Set Active", "Actor", "An inactive actor is not drawn nor updated", { Pin("Target", PinType::Actor), Pin("Active", PinType::Bool, "true") }, {},
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _a->SetActive(Bool(_r.InBool("Active"))); });
		Action("Actor.Destroy", "Destroy Actor", "Actor", "Removed at the end of the frame", { Pin("Target", PinType::Actor) }, {},
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _a->Destroy(); });
		Pure("Actor.GetName", "Get Name", "Actor", "", { Pin("Target", PinType::Actor) }, { Pin("Name", PinType::String) },
			[](NodeRun& _r) { if (Actor* _a = _r.InActor("Target")) _r.Out("Name", _a->GetName().StdString()); });
		Pure("Actor.IsValid", "Is Valid", "Actor", "Whether the actor exists (not destroyed)", { Pin("Actor", PinType::Actor) }, { Pin("Valid", PinType::Bool) },
			[](NodeRun& _r) { _r.Out("Valid", _r.InActor("Actor") != nullptr); });
		Pure("Actor.FindByName", "Find Actor By Name", "Actor", "The first actor of the level with this name", { Pin("Name", PinType::String, "Player") }, { Pin("Actor", PinType::Actor) },
			[](NodeRun& _r)
			{
				Actor* _found = nullptr;
				const std::string _name = _r.InString("Name");
				if (Level* _level = _r.GetLevel())
					for (const std::unique_ptr<Actor>& _actor : _level->GetActorManager().GetActors())
						if (_actor->GetName().StdString() == _name && !_actor->IsPendingDestroy()) { _found = _actor.get(); break; }
				_r.Out("Actor", _found);
			});
		Pure("Actor.Distance", "Distance To", "Actor", "Between two actors", { Pin("Target", PinType::Actor), Pin("Other", PinType::Actor) }, { Pin("Distance", PinType::Float) },
			[](NodeRun& _r)
			{
				Actor* _a = _r.InActor("Target");
				Actor* _b = _r.InActor("Other");
				_r.Out("Distance", _a && _b ? static_cast<float>((_b->GetPosition() - _a->GetPosition()).Length()) : 0.f);
			});
		Action("Actor.Spawn", "Spawn Actor", "Actor", "An actor of a class (EmptyActor, CameraActor, or one of your scripts) at a place",
			{ Pin("Class", PinType::String, "EmptyActor"), Pin("Position", PinType::Vector2) }, { Pin("Spawned", PinType::Actor) },
			[](NodeRun& _r)
			{
				Actor* _spawned = nullptr;
				const std::string _class = _r.InString("Class");
				const ClassInfo* _info = ClassRegistry::Find(_class);
				if (_info && _info->factory && _r.GetLevel())
				{
					_spawned = _info->factory(_r.GetLevel());
					if (_spawned) _spawned->SetPosition(_r.InVector("Position"));
				}
				else LOG_WARNING("Blueprint: Spawn Actor: no actor class \"" + _class + "\"");
				_r.Out("Spawned", _spawned);
			});

		// ---- Game
		Action("Game.Print", "Print", "Game", "Writes in the Console", { Pin("Text", PinType::String, "Hello") }, {},
			[](NodeRun& _r) { LOG_MSG("[" + _r.Instance().GetName() + "] " + _r.InString("Text")); });
		Pure("Game.Time", "Get Time", "Game", "Seconds since the blueprint started", {}, { Pin("Seconds", PinType::Float) },
			[](NodeRun& _r) { _r.Out("Seconds", _r.Time()); });
		Pure("Game.DeltaSeconds", "Get Delta Seconds", "Game", "Length of this frame", {}, { Pin("Seconds", PinType::Float) },
			[](NodeRun& _r) { _r.Out("Seconds", _r.DeltaSeconds()); });
		Pure("Input.IsHeld", "Is Input Held", "Input", "Whether a key or button is held now: \"Space\", \"A\", \"MouseLeft\"...", { Pin("Input", PinType::String, "Space") },
			{ Pin("Held", PinType::Bool) }, [](NodeRun& _r)
			{
				const auto _found = _r.Instance().held.find(_r.Literal("Input"));
				_r.Out("Held", _found != _r.Instance().held.end() && _found->second);
			});

		// ---- Effects
		Action("Effects.SpawnParticles", "Spawn Particles", "Effects", "A particle effect at a place (one-shot effects remove themselves)",
			{ EnumPin("Preset", "4", PRESETS), Pin("Position", PinType::Vector2) }, {},
			[](NodeRun& _r) { if (Level* _l = _r.GetLevel()) _l->SpawnParticles(static_cast<ParticlePreset>(std::clamp(_r.InInt("Preset"), 0, 8)), _r.InVector("Position")); });
		Action("Effects.Stain", "Stain", "Effects", "Marks the actors with a StainComponent around a place (burns, blood, paint)",
			{ Pin("Position", PinType::Vector2), Pin("Radius", PinType::Float, "24"), Pin("Color", PinType::Color, "30, 22, 18, 170"), EnumPin("Style", "2", { "Soft", "Splat", "Scorch" }) }, {},
			[](NodeRun& _r)
			{
				StainComponent::StainAt(_r.GetLevel(), _r.InVector("Position"), _r.InFloat("Radius"), _r.InColor("Color"), static_cast<StainStyle>(std::clamp(_r.InInt("Style"), 0, 2)));
			});
	}

	// ---- Nodes made from the reflection: Get / Set every property, Call every function, of every component and actor class.

	PinType PinOf(const PropertyType& _type)
	{
		switch (_type)
		{
		case PropertyType::Bool:	return PinType::Bool;
		case PropertyType::Int:
		case PropertyType::Flags:
		case PropertyType::Enum:	return PinType::Int;
		case PropertyType::Float:	return PinType::Float;
		case PropertyType::Vector2:	return PinType::Vector2;
		case PropertyType::Color:	return PinType::Color;
		default:					return PinType::String;
		}
	}

	bool Usable(const PropertyInfo& _property)
	{
		static const char* HIDDEN[] = { "animations", "graph", "bindings", "startAnimation" };	// edited by their own windows
		if (_property.type == PropertyType::Events || _property.type == PropertyType::InputBindings || _property.type == PropertyType::MessageBindings) return false;
		for (const char* _name : HIDDEN) if (_property.name == _name) return false;
		return true;
	}

	BpValue FromProperty(const PropertyValue& _value)
	{
		return std::visit([](const auto& _v) -> BpValue { return _v; }, _value);
	}

	std::optional<PropertyValue> ToProperty(const BpValue& _value, const PropertyType& _type)
	{
		const BpValue _converted = BlueprintValues::Convert(_value, PinOf(_type));
		switch (_converted.index())
		{
		case 0: return std::get<bool>(_converted);
		case 1: return std::get<int>(_converted);
		case 2: return std::get<float>(_converted);
		case 3: return std::get<std::string>(_converted);
		case 4: return std::get<FVector2>(_converted);
		case 5: return std::get<Color>(_converted);
		default: return std::nullopt;
		}
	}

	bool IsA(std::string _class, const std::string& _base)
	{
		for (int _i = 0; _i < 32 && !_class.empty(); ++_i)
		{
			if (_class == _base) return true;
			const ClassInfo* _info = ClassRegistry::Find(_class);
			if (!_info) return false;
			_class = _info->parentName;
		}
		return false;
	}

	// The actor itself or its component of this class.
	IGameObject* ObjectOf(Actor* _actor, const std::string& _class)
	{
		if (!_actor) return nullptr;
		if (IsA(ClassNameOf(*_actor), _class)) return _actor;
		for (const std::unique_ptr<Component>& _component : _actor->GetComponents())
			if (IsA(ClassNameOf(*_component), _class)) return _component.get();
		return nullptr;
	}

	const PropertyInfo* FindProperty(const std::string& _class, const std::string& _name)
	{
		const ClassInfo* _info = ClassRegistry::Find(_class);
		if (!_info) return nullptr;
		for (const PropertyInfo* _property : _info->GetProperties()) if (_property->name == _name) return _property;
		return nullptr;
	}

	const FunctionInfo* FindFunction(const std::string& _class, const std::string& _name)
	{
		const ClassInfo* _info = ClassRegistry::Find(_class);
		if (!_info) return nullptr;
		for (const FunctionInfo* _function : _info->GetFunctions()) if (_function->name == _name) return _function;
		return nullptr;
	}

	const NodeDesc* MakeGet(const std::string& _class, const PropertyInfo& _property)
	{
		const std::string _id = "Get." + _class + "." + _property.name;
		auto& _slot = Generated()[_id];
		if (_slot) return _slot.get();
		_slot = std::make_unique<NodeDesc>();
		NodeDesc& _node = *_slot;
		_node.id = _id; _node.title = "Get " + _property.name; _node.category = _class; _node.pure = true;
		_node.tooltip = "The " + _property.name + " of the " + _class + " of Target (no wire: this actor)";
		_node.inputs = { Pin("Target", PinType::Actor) };
		_node.outputs = { { _property.name, PinOf(_property.type), "", _property.type == PropertyType::Enum ? _property.flagNames : std::vector<std::string>{} } };
		const std::string _name = _property.name;
		_node.run = [_class, _name](NodeRun& _r)
		{
			const PropertyInfo* _p = FindProperty(_class, _name);
			IGameObject* _object = ObjectOf(_r.InActor("Target"), _class);
			if (_p && _object) _r.Out(_name, FromProperty(_p->get(_object)));
		};
		return &_node;
	}

	const NodeDesc* MakeSet(const std::string& _class, const PropertyInfo& _property)
	{
		const std::string _id = "Set." + _class + "." + _property.name;
		auto& _slot = Generated()[_id];
		if (_slot) return _slot.get();
		_slot = std::make_unique<NodeDesc>();
		NodeDesc& _node = *_slot;
		const PinType _type = PinOf(_property.type);
		_node.id = _id; _node.title = "Set " + _property.name; _node.category = _class;
		_node.tooltip = "Changes the " + _property.name + " of the " + _class + " of Target (no wire: this actor)";
		_node.inputs = { ExecPin(), Pin("Target", PinType::Actor),
			{ _property.name, _type, _type == PinType::Color ? "255, 255, 255, 255" : "", _property.type == PropertyType::Enum ? _property.flagNames : std::vector<std::string>{} } };
		_node.outputs = { ExecPin("Then") };
		const std::string _name = _property.name;
		_node.run = [_class, _name](NodeRun& _r)
		{
			const PropertyInfo* _p = FindProperty(_class, _name);
			IGameObject* _object = ObjectOf(_r.InActor("Target"), _class);
			if (_p && _object && !_p->readOnly)
				if (const std::optional<PropertyValue> _value = ToProperty(_r.In(_name), _p->type)) _p->set(_object, *_value);
			_r.Then();
		};
		return &_node;
	}

	const NodeDesc* MakeCall(const std::string& _class, const FunctionInfo& _function)
	{
		const std::string _id = "Call." + _class + "." + _function.name;
		auto& _slot = Generated()[_id];
		if (_slot) return _slot.get();
		_slot = std::make_unique<NodeDesc>();
		NodeDesc& _node = *_slot;
		_node.id = _id; _node.title = _function.name; _node.category = _class;
		_node.tooltip = "Calls " + _function.name + " on the " + _class + " of Target (no wire: this actor)";
		_node.inputs = { ExecPin(), Pin("Target", PinType::Actor) };
		if (_function.hasArgument) _node.inputs.push_back(Pin("Argument", PinOf(_function.argumentType), PinOf(_function.argumentType) == PinType::Color ? "255, 255, 255, 255" : ""));
		_node.outputs = { ExecPin("Then") };
		const std::string _name = _function.name;
		const bool _hasArgument = _function.hasArgument;
		const PropertyType _argumentType = _function.argumentType;
		_node.run = [_class, _name, _hasArgument, _argumentType](NodeRun& _r)
		{
			const FunctionInfo* _f = FindFunction(_class, _name);
			IGameObject* _object = ObjectOf(_r.InActor("Target"), _class);
			if (_f && _object && _f->call)
			{
				if (_hasArgument)
				{
					if (const std::optional<PropertyValue> _value = ToProperty(_r.In("Argument"), _argumentType)) _f->call(_object, &*_value);
				}
				else _f->call(_object, nullptr);
			}
			_r.Then();
		};
		return &_node;
	}

	const NodeDesc* MakeVariable(const BlueprintVariable& _variable, const bool _set)
	{
		const std::string _id = std::string(_set ? "Var.Set." : "Var.Get.") + _variable.name;
		const std::string _key = _id + "#" + std::to_string(_variable.type);	// the type may change: one node kind per type
		auto& _slot = Generated()[_key];
		if (_slot) return _slot.get();
		_slot = std::make_unique<NodeDesc>();
		NodeDesc& _node = *_slot;
		const PinType _type = static_cast<PinType>(_variable.type);
		const std::string _name = _variable.name;
		_node.id = _id; _node.category = "Variables";
		if (_set)
		{
			_node.title = "Set " + _name;
			_node.inputs = { ExecPin(), Pin(_name, _type, _type == PinType::Color ? "255, 255, 255, 255" : "") };
			_node.outputs = { ExecPin("Then"), Pin(_name, _type) };
			_node.run = [_name](NodeRun& _r)
			{
				_r.Instance().SetVariable(_name, _r.In(_name));
				_r.Out(_name, _r.Instance().GetVariable(_name));
				_r.Then();
			};
		}
		else
		{
			_node.title = _name;
			_node.pure = true;
			_node.outputs = { Pin(_name, _type) };
			_node.run = [_name](NodeRun& _r) { _r.Out(_name, _r.Instance().GetVariable(_name)); };
		}
		return &_node;
	}

	std::vector<std::string> ReflectedClasses()
	{
		std::vector<std::string> _names;
		for (const ClassInfo* _info : ClassRegistry::GetComponentClasses()) _names.push_back(_info->name);
		for (const ClassInfo* _info : ClassRegistry::GetActorClasses()) _names.push_back(_info->name);
		return _names;
	}
}

const NodeDesc* BlueprintNodes::Find(const std::string& _id, const BlueprintAsset* _asset)
{
	BuildLibrary();
	for (const std::unique_ptr<NodeDesc>& _node : Library()) if (_node->id == _id) return _node.get();

	// "Var.Get.name", "Get.Class.property", "Set.Class.property", "Call.Class.function".
	if (_id.rfind("Var.Get.", 0) == 0 || _id.rfind("Var.Set.", 0) == 0)
	{
		const BlueprintVariable* _variable = _asset ? _asset->FindVariable(_id.substr(8)) : nullptr;
		return _variable ? MakeVariable(*_variable, _id[4] == 'S') : nullptr;
	}
	const size_t _first = _id.find('.'), _second = _id.find('.', _first + 1);
	if (_first == std::string::npos || _second == std::string::npos) return nullptr;
	const std::string _kind = _id.substr(0, _first), _class = _id.substr(_first + 1, _second - _first - 1), _member = _id.substr(_second + 1);
	if (_kind == "Get" || _kind == "Set")
	{
		const PropertyInfo* _property = FindProperty(_class, _member);
		if (!_property || !Usable(*_property)) return nullptr;
		return _kind == "Get" ? MakeGet(_class, *_property) : MakeSet(_class, *_property);
	}
	if (_kind == "Call")
	{
		const FunctionInfo* _function = FindFunction(_class, _member);
		return _function ? MakeCall(_class, *_function) : nullptr;
	}
	return nullptr;
}

std::vector<const NodeDesc*> BlueprintNodes::Palette(const BlueprintAsset* _asset)
{
	BuildLibrary();
	std::vector<const NodeDesc*> _all;
	for (const std::unique_ptr<NodeDesc>& _node : Library()) _all.push_back(_node.get());
	if (_asset)
		for (const BlueprintVariable& _variable : _asset->variables)
		{
			_all.push_back(MakeVariable(_variable, false));
			_all.push_back(MakeVariable(_variable, true));
		}
	for (const std::string& _class : ReflectedClasses())
	{
		const ClassInfo* _info = ClassRegistry::Find(_class);
		if (!_info) continue;
		for (const PropertyInfo* _property : _info->GetProperties())
		{
			if (!Usable(*_property)) continue;
			_all.push_back(MakeGet(_class, *_property));
			if (!_property->readOnly) _all.push_back(MakeSet(_class, *_property));
		}
		for (const FunctionInfo* _function : _info->GetFunctions()) _all.push_back(MakeCall(_class, *_function));
	}
	return _all;
}
