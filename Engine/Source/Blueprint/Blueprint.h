#pragma once
#include "Utilities/Math/Vector2D.h"
#include "Utilities/Debug/Color.h"
#include "Utilities/System/Event.h"
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

// Blueprints: visual scripts made of nodes linked by wires, like the ones of Unreal Engine.
//   - an asset Content/Blueprints/Name.kbp holds the graph: its variables, nodes and links (made in the Blueprint Editor);
//   - a BlueprintComponent on an actor runs it: its events (BeginPlay, Tick, Input, custom ones) start chains of nodes;
//   - white "exec" wires say what runs after what, colored wires carry values (pulled when a node needs them).
// Nodes come from a library (events, flow, math, actors...) plus, made from the reflection, a Get / Set node for every property and a
// Call node for every function of every component and actor class: what the Inspector shows, a blueprint can use.

namespace Krampus
{
	class Actor;
	class Level;
	class BlueprintInstance;

	/// <summary>Type of a pin (saved as an int: never reorder).</summary>
	enum class PinType
	{
		Exec,		// what runs next
		Bool,
		Int,
		Float,
		String,
		Vector2,
		Color,
		Actor
	};

	/// <summary>A value carried by a wire.</summary>
	using BpValue = std::variant<bool, int, float, std::string, FVector2, Color, Actor*>;

	namespace BlueprintValues
	{
		const char* TypeName(const PinType& _type);
		BpValue Default(const PinType& _type);

		/// <summary>The value as another type: numbers into each other, anything into text, a number into a bool...</summary>
		BpValue Convert(const BpValue& _value, const PinType& _type);

		/// <returns>True when a wire may go from an output of type _from to an input of type _to.</returns>
		bool CanConnect(const PinType& _from, const PinType& _to);

		/// <summary>A value written in a file or typed in a node ("1.5", "true", "3, 4", "255, 0, 0, 255").</summary>
		BpValue Parse(const std::string& _text, const PinType& _type);
		std::string Format(const BpValue& _value);

		/// <summary>Readable text of a value (Print).</summary>
		std::string ToText(const BpValue& _value);
	}

	// ================= Asset =================

	struct BlueprintVariable
	{
		std::string		name;
		int				type	= static_cast<int>(PinType::Float);
		std::string		value;		// starting value, as text
	};

	struct BlueprintNode
	{
		int										id		= 0;
		std::string								type;		// id of its NodeDesc ("Flow.Branch", "Var.Get.score", "Set.SpriteComponent.zOrder"...)
		FVector2								position;
		std::map<std::string, std::string>		values;		// what is typed in its inputs that have no wire
	};

	struct BlueprintLink
	{
		int				fromNode	= 0;
		std::string		fromPin;
		int				toNode		= 0;
		std::string		toPin;
	};

	/// <summary>The graph of a blueprint, as saved in Content/Blueprints/Name.kbp.</summary>
	struct BlueprintAsset
	{
		static constexpr const char*	EXTENSION	= ".kbp";

		std::vector<BlueprintVariable>	variables;
		std::vector<BlueprintNode>		nodes;
		std::vector<BlueprintLink>		links;
		int								nextId		= 1;

		BlueprintNode* FindNode(const int& _id);
		const BlueprintNode* FindNode(const int& _id) const;
		const BlueprintVariable* FindVariable(const std::string& _name) const;

		static std::string Encode(const BlueprintAsset& _asset);
		static BlueprintAsset Decode(const std::string& _text);

		/// <returns>Content/Blueprints.</returns>
		static std::filesystem::path Folder();
		static std::filesystem::path PathOf(const std::string& _name);
		static bool Exists(const std::string& _name);

		/// <returns>The blueprint (read once, then kept), or null when there is no such file.</returns>
		static std::shared_ptr<const BlueprintAsset> Load(const std::string& _name);

		/// <summary>Writes the file and says it (OnChanged).</summary>
		static bool Save(const std::string& _name, const BlueprintAsset& _asset);

		/// <returns>Every blueprint of Content/Blueprints (names with their folders), sorted.</returns>
		static std::vector<std::string> List();

		static Event<const std::string&>& OnChanged();
	};

	// ================= Nodes =================

	struct PinDesc
	{
		std::string					name;
		PinType						type		= PinType::Exec;
		std::string					value;		// default value of an input, as text
		std::vector<std::string>	enumNames;	// an Int input shown as a list of names
	};

	class NodeRun;

	/// <summary>What a kind of node is: its pins and what it does.</summary>
	struct NodeDesc
	{
		std::string					id;
		std::string					title;
		std::string					category;
		std::string					tooltip;
		std::vector<PinDesc>		inputs;
		std::vector<PinDesc>		outputs;
		bool						pure		= false;	// no exec pin: computed each time a value is pulled from it
		bool						event		= false;	// starts chains (BeginPlay, Tick...)
		std::function<void(NodeRun&)>	run;

		int InputIndex(const std::string& _name) const;
		int OutputIndex(const std::string& _name) const;
	};

	namespace BlueprintNodes
	{
		/// <returns>The kind of node of this id (variable nodes need the asset that has the variable), or null if unknown.</returns>
		const NodeDesc* Find(const std::string& _id, const BlueprintAsset* _asset);

		/// <returns>Every node that can be added to the asset, for the Blueprint Editor's list.</returns>
		std::vector<const NodeDesc*> Palette(const BlueprintAsset* _asset);
	}

	/// <summary>What a node sees while it runs: its inputs, its outputs, the actor, the time.</summary>
	class NodeRun
	{
		BlueprintInstance&	instance;
		int					node;
		int					entered;	// exec input it was entered by (-1: an event, or a pure node)

	public:
		std::vector<int>	next;		// exec outputs to follow, in order

		NodeRun(BlueprintInstance& _instance, const int& _node, const int& _entered) : instance(_instance), node(_node), entered(_entered) { }

		BpValue In(const std::string& _pin);
		float InFloat(const std::string& _pin);
		int InInt(const std::string& _pin);
		bool InBool(const std::string& _pin);
		std::string InString(const std::string& _pin);
		FVector2 InVector(const std::string& _pin);
		Color InColor(const std::string& _pin);
		/// <returns>The actor of the input (no wire: the blueprint's own actor), null if it does not exist anymore.</returns>
		Actor* InActor(const std::string& _pin);

		/// <summary>The text typed in an input (an event's settings: never pulled through a wire).</summary>
		std::string Literal(const std::string& _pin) const;

		void Out(const std::string& _pin, const BpValue& _value);

		/// <summary>Follows this exec output once the node is done (call it for each one, in order).</summary>
		void Then(const std::string& _pin = "Then");

		/// <summary>Runs the chain of this exec output now, before going on (loops).</summary>
		void RunNow(const std::string& _pin);

		/// <summary>Follows this exec output after a delay (Delay). A node already waiting is not started again.</summary>
		void Wait(const float& _seconds, const std::string& _pin);

		/// <returns>The name of the exec input the node was entered by ("In", "Reset"...).</returns>
		std::string EnteredBy() const;

		/// <summary>A number the node keeps between runs (Do Once, Flip Flop).</summary>
		float& State();

		Actor* Self() const;
		Level* GetLevel() const;
		float DeltaSeconds() const;
		float Time() const;
		BlueprintInstance& Instance() { return instance; }
	};

	/// <summary>One blueprint running for one actor: the graph made ready to run, its variables and what waits (Delay).</summary>
	class BlueprintInstance
	{
	public:
		struct Compiled
		{
			int										id			= 0;
			const NodeDesc*							desc		= nullptr;
			const BlueprintNode*					source		= nullptr;
			std::vector<BpValue>					literals;	// per input: the typed value
			std::vector<std::pair<int, int>>		inputs;		// per input: (node, output) of its wire, or (-1, -1)
			std::vector<std::vector<std::pair<int, int>>>	exec;	// per output: (node, input) its exec wires go to
			std::vector<BpValue>					outputs;	// per output: the last value
			float									state		= 0.f;
			float									lastRun		= -1e9f;
		};

		/// <summary>Makes the graph ready to run for _self. Problems (unknown nodes, wires between pins that do not fit) go to _errors.</summary>
		bool Compile(const std::shared_ptr<const BlueprintAsset>& _asset, Actor* _self, const std::string& _name, std::vector<std::string>* _errors = nullptr);

		/// <summary>Starts every event node of this kind (_key: its Name / Input value, empty = any).</summary>
		void FireEvent(const std::string& _type, const std::string& _key = "");

		/// <summary>Moves the time on: what waits (Delay), then the Event Tick nodes.</summary>
		void Tick(const float& _deltaTime);

		bool IsReady() const { return !nodes.empty(); }
		const std::string& GetName() const { return name; }
		const std::shared_ptr<const BlueprintAsset>& GetAsset() const { return asset; }

		/// <returns>The Event Input names its event nodes listen to.</returns>
		std::vector<std::string> InputEvents() const;
		std::vector<std::string> InputsHeldChecks() const;

		BpValue GetVariable(const std::string& _name) const;
		void SetVariable(const std::string& _name, const BpValue& _value);

		/// <returns>Seconds since the node of this id last ran (the editor lights the nodes that run). Very big: never.</returns>
		float SinceRun(const int& _id) const;

		/// <summary>Whether an input ("Space", "A", "MouseLeft"...) is held: kept up to date by the BlueprintComponent.</summary>
		std::unordered_map<std::string, bool>	held;

	private:
		std::string											name;
		std::shared_ptr<const BlueprintAsset>				asset;
		Actor*												self		= nullptr;
		std::vector<Compiled>								nodes;
		std::unordered_map<int, int>						indexOf;	// node id -> index
		std::vector<BpValue>								variables;
		std::unordered_map<std::string, int>				variableIndex;

		struct Waiting { float remaining; int node; int output; };
		std::vector<Waiting>								waiting;
		float												time		= 0.f;
		float												delta		= 0.f;
		int													steps		= 0;
		int													depth		= 0;
		bool												stopped		= false;	// a runaway loop stopped the blueprint

		void ExecuteFrom(const int& _node, const int& _entered);
		void FollowOutput(const int& _node, const int& _output);
		BpValue Pull(const int& _node, const int& _input);
		void Evaluate(const int& _node);
		bool Budget();

		friend class NodeRun;
	};
}
