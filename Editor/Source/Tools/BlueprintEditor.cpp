#include "EditorContext.h"
#include "EditorTheme.h"
#include "Actors/Actor.h"
#include "Blueprint/Blueprint.h"
#include "Components/BlueprintComponent.h"
#include "Managers/InputManager.h"
#include <imgui.h>
#include <cmath>
#include <set>

using namespace Krampus;

// The Blueprint Editor: the graph of a blueprint of Content/Blueprints.
//   - right click (or drop a wire on nothing): the list of nodes, with a search;
//   - drag from a pin to another: a wire (white: what runs next; colored: a value); Alt + click on a pin: its wires go;
//   - a value pin without wire is typed in the node; an Actor pin without wire is the blueprint's own actor (Self);
//   - drag nodes to move them, drag on nothing to select several, Delete, Ctrl+D to copy, Ctrl+Z / Ctrl+Y, Ctrl+S to save;
//   - in Play, with the actor selected, the nodes that run light up.

namespace
{
	// Sizes in graph units (multiplied by the zoom on the screen).
	constexpr float		HEADER		= 26.f;
	constexpr float		ROW			= 24.f;
	constexpr float		PIN_RADIUS	= 5.f;
	constexpr float		PADDING		= 10.f;
	constexpr float		MIN_WIDTH	= 140.f;
	constexpr size_t	MAX_UNDO	= 100;

	struct PinRef
	{
		int			node	= -1;
		std::string	pin;
		bool		output	= false;
		bool Valid() const { return node >= 0; }
	};

	// Where a node and its pins are on the screen this frame.
	struct Layout
	{
		int					id		= 0;
		const NodeDesc*		desc	= nullptr;
		ImVec2				min, max;
		std::vector<ImVec2>	inputs, outputs;
	};

	bool						g_open			= false;
	std::string					g_name;				// the blueprint being edited
	BlueprintAsset				g_asset;			// its graph, saved on Save / Play / hot reload
	bool						g_dirty			= false;
	std::vector<BlueprintAsset>	g_undo, g_redo;
	std::string					g_lastChange;		// a continued change (a drag, typing) is one undo step
	std::vector<std::string>	g_errors;
	bool						g_checked		= false;

	ImVec2						g_pan			= ImVec2(80.f, 80.f);
	float						g_zoom			= 1.f;
	std::set<int>				g_selected;
	bool						g_dragging		= false;
	bool						g_dragMoved		= false;
	PinRef						g_wire;				// a wire being drawn from this pin
	bool						g_boxing		= false;
	ImVec2						g_boxStart;
	ImVec2						g_paletteAt;		// graph position for the node picked in the list
	PinRef						g_paletteFrom;		// a wire dropped on nothing: the new node gets it
	char						g_search[64]	= "";
	bool						g_focusSearch	= false;
	char						g_newName[64]	= "NewBlueprint";
	int							g_renaming		= -1;
	char						g_renameBuffer[64] = "";

	ImVec2 operator+(const ImVec2& _a, const ImVec2& _b) { return ImVec2(_a.x + _b.x, _a.y + _b.y); }
	ImVec2 operator-(const ImVec2& _a, const ImVec2& _b) { return ImVec2(_a.x - _b.x, _a.y - _b.y); }
	ImVec2 operator*(const ImVec2& _a, const float _f) { return ImVec2(_a.x * _f, _a.y * _f); }

	ImU32 PinColor(const PinType& _type)
	{
		switch (_type)
		{
		case PinType::Exec:		return IM_COL32(235, 235, 235, 255);
		case PinType::Bool:		return IM_COL32(200, 50, 50, 255);
		case PinType::Int:		return IM_COL32(40, 210, 160, 255);
		case PinType::Float:	return IM_COL32(150, 225, 70, 255);
		case PinType::String:	return IM_COL32(240, 90, 210, 255);
		case PinType::Vector2:	return IM_COL32(245, 200, 50, 255);
		case PinType::Color:	return IM_COL32(80, 120, 255, 255);
		default:				return IM_COL32(60, 170, 255, 255);	// Actor
		}
	}

	ImU32 HeaderColor(const NodeDesc* _desc)
	{
		if (!_desc) return IM_COL32(150, 30, 30, 255);
		const std::string& _c = _desc->category;
		if (_desc->event)		return IM_COL32(140, 30, 30, 255);
		if (_c == "Flow")		return IM_COL32(85, 88, 96, 255);
		if (_c == "Variables")	return _desc->pure ? IM_COL32(60, 110, 70, 255) : IM_COL32(45, 95, 60, 255);
		if (_c == "Math" || _c == "Logic" || _c == "Vector" || _c == "String") return IM_COL32(50, 100, 70, 255);
		if (_c == "Actor" || _c == "Game" || _c == "Input" || _c == "Effects") return IM_COL32(40, 80, 150, 255);
		if (_desc->id.rfind("Call.", 0) == 0) return IM_COL32(95, 60, 150, 255);
		return IM_COL32(30, 110, 120, 255);	// Get / Set of a class
	}

	// ---- Changes and saving

	void Change(const std::string& _key = "")
	{
		g_dirty = true;
		g_checked = false;
		if (!_key.empty() && _key == g_lastChange) return;
		g_lastChange = _key;
		g_undo.push_back(g_asset);
		if (g_undo.size() > MAX_UNDO) g_undo.erase(g_undo.begin());
		g_redo.clear();
	}

	void Check()
	{
		g_errors.clear();
		BlueprintInstance _test;
		_test.Compile(std::make_shared<const BlueprintAsset>(g_asset), nullptr, g_name, &g_errors);
		bool _event = false;
		for (const BlueprintNode& _node : g_asset.nodes)
			if (const NodeDesc* _desc = BlueprintNodes::Find(_node.type, &g_asset)) _event = _event || _desc->event;
		if (!_event) g_errors.push_back("no event node: nothing will ever run (add Event BeginPlay or Event Tick)");
		g_checked = true;
	}

	bool SaveNow()
	{
		if (g_name.empty()) return false;
		Check();
		if (!BlueprintAsset::Save(g_name, g_asset))
		{
			LOG_ERROR("Blueprint Editor: cannot write " + BlueprintAsset::PathOf(g_name).generic_string());
			return false;
		}
		g_dirty = false;
		LOG_MSG("Blueprint " + g_name + " saved" + (g_errors.empty() ? "" : " (" + std::to_string(g_errors.size()) + " problem(s))"));
		return true;
	}

	void Open(const std::string& _name)
	{
		if (g_dirty) SaveNow();	// the one being edited is not lost
		const std::shared_ptr<const BlueprintAsset> _asset = BlueprintAsset::Load(_name);
		if (!_asset) return;
		g_name = _name;
		g_asset = *_asset;
		g_dirty = false;
		g_undo.clear();
		g_redo.clear();
		g_selected.clear();
		g_wire = PinRef();
		g_pan = ImVec2(80.f, 80.f);
		g_zoom = 1.f;
		Check();
	}

	int AddNode(const NodeDesc& _desc, const ImVec2& _at)
	{
		BlueprintNode _node;
		_node.id = g_asset.nextId++;
		_node.type = _desc.id;
		_node.position = FVector2(std::round(_at.x), std::round(_at.y));
		g_asset.nodes.push_back(_node);
		return _node.id;
	}

	std::string CreateBlueprint(const std::string& _wanted)
	{
		std::string _name = _wanted.empty() ? "NewBlueprint" : _wanted;
		for (int _n = 1; BlueprintAsset::Exists(_name); ++_n) _name = (_wanted.empty() ? "NewBlueprint" : _wanted) + std::to_string(_n);
		BlueprintAsset _asset;	// like Unreal: a new blueprint starts with its two main events
		if (const NodeDesc* _begin = BlueprintNodes::Find("Event.BeginPlay", nullptr)) _asset.nodes.push_back({ _asset.nextId++, _begin->id, FVector2(0.f, 0.f), {} });
		if (const NodeDesc* _tick = BlueprintNodes::Find("Event.Tick", nullptr)) _asset.nodes.push_back({ _asset.nextId++, _tick->id, FVector2(0.f, 160.f), {} });
		if (!BlueprintAsset::Save(_name, _asset)) return "";
		LOG_MSG("Blueprint Editor: created " + BlueprintAsset::PathOf(_name).generic_string());
		return _name;
	}

	// ---- Wires

	const PinDesc* PinOf(const PinRef& _ref)
	{
		const BlueprintNode* _node = g_asset.FindNode(_ref.node);
		const NodeDesc* _desc = _node ? BlueprintNodes::Find(_node->type, &g_asset) : nullptr;
		if (!_desc) return nullptr;
		const auto& _pins = _ref.output ? _desc->outputs : _desc->inputs;
		for (const PinDesc& _pin : _pins) if (_pin.name == _ref.pin) return &_pin;
		return nullptr;
	}

	bool Connect(PinRef _a, PinRef _b)
	{
		if (!_a.Valid() || !_b.Valid() || _a.node == _b.node || _a.output == _b.output) return false;
		if (!_a.output) std::swap(_a, _b);	// _a: the output
		const PinDesc* _out = PinOf(_a);
		const PinDesc* _in = PinOf(_b);
		if (!_out || !_in || !BlueprintValues::CanConnect(_out->type, _in->type)) return false;

		Change();
		// A value input takes one wire; an exec output goes to one place (like Unreal).
		if (_in->type != PinType::Exec)
			std::erase_if(g_asset.links, [&](const BlueprintLink& _l) { return _l.toNode == _b.node && _l.toPin == _b.pin; });
		else
			std::erase_if(g_asset.links, [&](const BlueprintLink& _l) { return _l.fromNode == _a.node && _l.fromPin == _a.pin; });
		g_asset.links.push_back({ _a.node, _a.pin, _b.node, _b.pin });
		return true;
	}

	void BreakLinks(const PinRef& _pin)
	{
		Change();
		std::erase_if(g_asset.links, [&](const BlueprintLink& _l)
		{
			return _pin.output ? (_l.fromNode == _pin.node && _l.fromPin == _pin.pin) : (_l.toNode == _pin.node && _l.toPin == _pin.pin);
		});
	}

	void DeleteSelected()
	{
		if (g_selected.empty()) return;
		Change();
		std::erase_if(g_asset.nodes, [](const BlueprintNode& _n) { return g_selected.contains(_n.id); });
		std::erase_if(g_asset.links, [](const BlueprintLink& _l) { return g_selected.contains(_l.fromNode) || g_selected.contains(_l.toNode); });
		g_selected.clear();
	}

	void DuplicateSelected()
	{
		if (g_selected.empty()) return;
		Change();
		std::unordered_map<int, int> _copyOf;
		const std::vector<BlueprintNode> _nodes = g_asset.nodes;
		for (const BlueprintNode& _node : _nodes)
		{
			if (!g_selected.contains(_node.id)) continue;
			BlueprintNode _copy = _node;
			_copy.id = g_asset.nextId++;
			_copy.position = FVector2(_node.position.x + 30.f, _node.position.y + 30.f);
			_copyOf[_node.id] = _copy.id;
			g_asset.nodes.push_back(_copy);
		}
		// The wires between copied nodes are copied too.
		const std::vector<BlueprintLink> _links = g_asset.links;
		for (const BlueprintLink& _link : _links)
			if (_copyOf.contains(_link.fromNode) && _copyOf.contains(_link.toNode))
				g_asset.links.push_back({ _copyOf[_link.fromNode], _link.fromPin, _copyOf[_link.toNode], _link.toPin });
		g_selected.clear();
		for (const auto& [_old, _new] : _copyOf) g_selected.insert(_new);
	}

	// ---- Typed values in the nodes

	std::string ValueOf(const BlueprintNode& _node, const PinDesc& _pin)
	{
		const auto _found = _node.values.find(_pin.name);
		return _found != _node.values.end() ? _found->second : _pin.value;
	}

	float WidgetWidth(const PinDesc& _pin)
	{
		switch (_pin.type)
		{
		case PinType::Bool:		return 22.f;
		case PinType::Color:	return 26.f;
		case PinType::Vector2:	return 120.f;
		case PinType::String:	return 100.f;
		case PinType::Actor:	return 36.f;	// "Self"
		default:				return _pin.enumNames.empty() ? 64.f : 96.f;
		}
	}

	// A widget for an input without wire; returns the new text when it was changed.
	bool DrawValueWidget(BlueprintNode& _node, const PinDesc& _pin, const float _width)
	{
		const std::string _text = ValueOf(_node, _pin);
		const std::string _key = std::to_string(_node.id) + "." + _pin.name;
		ImGui::PushID(_key.c_str());
		ImGui::SetNextItemWidth(_width);
		bool _changed = false;
		std::string _new = _text;
		switch (_pin.type)
		{
		case PinType::Bool:
		{
			bool _value = std::get<bool>(BlueprintValues::Parse(_text, PinType::Bool));
			if (ImGui::Checkbox("##v", &_value)) { _new = _value ? "true" : "false"; _changed = true; }
			break;
		}
		case PinType::Int:
		{
			int _value = std::get<int>(BlueprintValues::Parse(_text, PinType::Int));
			if (!_pin.enumNames.empty())
			{
				const int _index = std::clamp(_value, 0, static_cast<int>(_pin.enumNames.size()) - 1);
				if (ImGui::BeginCombo("##v", _pin.enumNames[_index].c_str()))
				{
					for (int _i = 0; _i < static_cast<int>(_pin.enumNames.size()); ++_i)
						if (ImGui::Selectable(_pin.enumNames[_i].c_str(), _i == _index)) { _new = std::to_string(_i); _changed = true; }
					ImGui::EndCombo();
				}
			}
			else if (ImGui::DragInt("##v", &_value)) { _new = std::to_string(_value); _changed = true; }
			break;
		}
		case PinType::Float:
		{
			float _value = std::get<float>(BlueprintValues::Parse(_text, PinType::Float));
			if (ImGui::DragFloat("##v", &_value, 0.1f, 0.f, 0.f, "%.3g")) { _new = BlueprintValues::Format(_value); _changed = true; }
			break;
		}
		case PinType::String:
		{
			char _buffer[256];
			std::snprintf(_buffer, sizeof(_buffer), "%s", _text.c_str());
			if (ImGui::InputText("##v", _buffer, sizeof(_buffer))) { _new = _buffer; _changed = true; }
			break;
		}
		case PinType::Vector2:
		{
			const FVector2 _v = std::get<FVector2>(BlueprintValues::Parse(_text, PinType::Vector2));
			float _values[2] = { _v.x, _v.y };
			if (ImGui::DragFloat2("##v", _values, 0.5f, 0.f, 0.f, "%.3g")) { _new = BlueprintValues::Format(FVector2(_values[0], _values[1])); _changed = true; }
			break;
		}
		case PinType::Color:
		{
			const Color _c = std::get<Color>(BlueprintValues::Parse(_text.empty() ? "255, 255, 255, 255" : _text, PinType::Color));
			float _values[4] = { _c.r / 255.f, _c.g / 255.f, _c.b / 255.f, _c.a / 255.f };
			if (ImGui::ColorEdit4("##v", _values, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar))
			{
				const auto _byte = [](const float _f) { return static_cast<std::uint8_t>(std::clamp(_f, 0.f, 1.f) * 255.f + 0.5f); };
				_new = BlueprintValues::Format(Color(_byte(_values[0]), _byte(_values[1]), _byte(_values[2]), _byte(_values[3])));
				_changed = true;
			}
			break;
		}
		case PinType::Actor:
			ImGui::AlignTextToFramePadding();
			ImGui::TextDisabled("Self");
			break;
		default:
			break;
		}
		if (_changed)
		{
			Change("value." + _key);
			_node.values[_pin.name] = _new;
		}
		ImGui::PopID();
		return _changed;
	}

	// ---- Left panel: list of blueprints, variables, problems

	void DrawSidePanel(EditorContext& _context)
	{
		// Live values: the selected actor runs this blueprint.
		const BlueprintComponent* _running = nullptr;
		if (_context.playing && _context.selected)
			if (const BlueprintComponent* _component = _context.selected->GetComponent<BlueprintComponent>())
				if (_component->blueprint == g_name && _component->IsRunning()) _running = _component;

		ImGui::SeparatorText("Variables");
		int _remove = -1;
		for (int _i = 0; _i < static_cast<int>(g_asset.variables.size()); ++_i)
		{
			BlueprintVariable& _variable = g_asset.variables[_i];
			ImGui::PushID(_i);
			const ImU32 _color = PinColor(static_cast<PinType>(_variable.type));
			ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(ImGui::GetCursorScreenPos().x + 5.f, ImGui::GetCursorScreenPos().y + ImGui::GetFrameHeight() * 0.5f), 5.f, _color);
			ImGui::Dummy(ImVec2(12.f, ImGui::GetFrameHeight()));
			ImGui::SameLine();

			// Name: applied when the field is left; the nodes and wires that use it follow.
			char _buffer[64];
			std::snprintf(_buffer, sizeof(_buffer), "%s", g_renaming == _i ? g_renameBuffer : _variable.name.c_str());
			ImGui::SetNextItemWidth(110.f);
			if (ImGui::InputText("##name", _buffer, sizeof(_buffer))) { g_renaming = _i; std::snprintf(g_renameBuffer, sizeof(g_renameBuffer), "%s", _buffer); }
			if (ImGui::IsItemDeactivatedAfterEdit() && g_renaming == _i)
			{
				const std::string _old = _variable.name, _new = g_renameBuffer;
				if (!_new.empty() && _new != _old && !g_asset.FindVariable(_new) && _new.find_first_of(".|") == std::string::npos)
				{
					Change();
					g_asset.variables[_i].name = _new;
					for (BlueprintNode& _node : g_asset.nodes)
						if (_node.type == "Var.Get." + _old || _node.type == "Var.Set." + _old)
						{
							_node.type = _node.type.substr(0, 8) + _new;
							if (_node.values.contains(_old)) { _node.values[_new] = _node.values[_old]; _node.values.erase(_old); }
						}
					for (BlueprintLink& _link : g_asset.links)	// the pins of its nodes are named like it
					{
						const BlueprintNode* _from = g_asset.FindNode(_link.fromNode);
						const BlueprintNode* _to = g_asset.FindNode(_link.toNode);
						if (_from && _from->type.rfind("Var.", 0) == 0 && _from->type.substr(8) == _new && _link.fromPin == _old) _link.fromPin = _new;
						if (_to && _to->type.rfind("Var.", 0) == 0 && _to->type.substr(8) == _new && _link.toPin == _old) _link.toPin = _new;
					}
				}
				g_renaming = -1;
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(78.f);
			const char* TYPES[] = { "Bool", "Int", "Float", "String", "Vector2", "Color", "Actor" };
			int _type = _variable.type - 1;
			if (ImGui::Combo("##type", &_type, TYPES, 7) && _type + 1 != _variable.type)
			{
				Change();
				g_asset.variables[_i].type = _type + 1;
				g_asset.variables[_i].value = BlueprintValues::Format(BlueprintValues::Default(static_cast<PinType>(_type + 1)));
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("x")) _remove = _i;

			// Starting value (live value in Play).
			ImGui::Indent(16.f);
			if (_running)
				ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::PlayMode), "= %s", BlueprintValues::ToText(_running->GetVariable(g_asset.variables[_i].name)).c_str());
			else if (static_cast<PinType>(g_asset.variables[_i].type) != PinType::Actor)
			{
				BlueprintNode _holder;	// the widget edits a node value: borrow one
				_holder.id = -1000 - _i;
				const PinDesc _pin{ "start", static_cast<PinType>(g_asset.variables[_i].type), g_asset.variables[_i].value, {} };
				if (DrawValueWidget(_holder, _pin, 180.f)) g_asset.variables[_i].value = _holder.values["start"];
			}
			if (ImGui::SmallButton("Get")) { Change(); AddNode(*BlueprintNodes::Find("Var.Get." + g_asset.variables[_i].name, &g_asset), (ImVec2(300.f, 200.f) - g_pan) * (1.f / g_zoom)); }
			ImGui::SameLine();
			if (ImGui::SmallButton("Set")) { Change(); AddNode(*BlueprintNodes::Find("Var.Set." + g_asset.variables[_i].name, &g_asset), (ImVec2(300.f, 240.f) - g_pan) * (1.f / g_zoom)); }
			ImGui::Unindent(16.f);
			ImGui::PopID();
		}
		if (_remove >= 0)
		{
			Change();
			const std::string _name = g_asset.variables[_remove].name;
			g_asset.variables.erase(g_asset.variables.begin() + _remove);
			std::set<int> _gone;
			for (const BlueprintNode& _node : g_asset.nodes)
				if (_node.type == "Var.Get." + _name || _node.type == "Var.Set." + _name) _gone.insert(_node.id);
			std::erase_if(g_asset.nodes, [&](const BlueprintNode& _n) { return _gone.contains(_n.id); });
			std::erase_if(g_asset.links, [&](const BlueprintLink& _l) { return _gone.contains(_l.fromNode) || _gone.contains(_l.toNode); });
			g_renaming = -1;
		}
		if (ImGui::Button("+ Variable", ImVec2(-1.f, 0.f)))
		{
			Change();
			std::string _name = "NewVar";
			for (int _n = 1; g_asset.FindVariable(_name); ++_n) _name = "NewVar" + std::to_string(_n);
			g_asset.variables.push_back({ _name, static_cast<int>(PinType::Float), "0" });
		}

		ImGui::SeparatorText("Check");
		if (ImGui::Button("Compile", ImVec2(-1.f, 0.f))) Check();
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Looks for problems (unknown nodes, wires that do not fit). Save does it too.");
		if (g_checked)
		{
			if (g_errors.empty()) ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::PlayButton), "No problem");
			for (const std::string& _error : g_errors)
				ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::ConsoleError), "- %s", _error.c_str());
		}
		if (_running) ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::PlayMode), "Running on %s: the nodes that run light up.", _context.selected->GetName().StdString().c_str());
		else if (_context.playing) ImGui::TextWrapped("Select an actor that runs this blueprint to see it work.");
	}

	// ---- The node list (right click, or a wire dropped on nothing)

	bool Fits(const NodeDesc& _desc, const PinRef& _from, const PinType& _fromType)
	{
		if (!_from.Valid()) return true;
		const auto& _pins = _from.output ? _desc.inputs : _desc.outputs;
		for (const PinDesc& _pin : _pins)
			if (_from.output ? BlueprintValues::CanConnect(_fromType, _pin.type) : BlueprintValues::CanConnect(_pin.type, _fromType)) return true;
		return false;
	}

	void DrawPalette()
	{
		if (!ImGui::BeginPopup("##palette")) return;

		const PinDesc* _fromPin = g_paletteFrom.Valid() ? PinOf(g_paletteFrom) : nullptr;
		const PinType _fromType = _fromPin ? _fromPin->type : PinType::Exec;
		ImGui::TextDisabled(_fromPin ? "Nodes that take this wire" : "Add a node");
		if (g_focusSearch) { ImGui::SetKeyboardFocusHere(); g_focusSearch = false; }
		ImGui::SetNextItemWidth(320.f);
		ImGui::InputTextWithHint("##search", "Search (Branch, Print, Set position, zOrder...)", g_search, sizeof(g_search));
		std::string _needle = g_search;
		std::transform(_needle.begin(), _needle.end(), _needle.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });

		const NodeDesc* _picked = nullptr;
		if (ImGui::BeginChild("##list", ImVec2(320.f, 380.f)))
		{
			std::map<std::string, std::vector<const NodeDesc*>> _byCategory;
			for (const NodeDesc* _desc : BlueprintNodes::Palette(&g_asset))
			{
				if (_fromPin && !Fits(*_desc, g_paletteFrom, _fromType)) continue;
				if (!_needle.empty())
				{
					std::string _hay = _desc->title + " " + _desc->category + " " + _desc->id;
					std::transform(_hay.begin(), _hay.end(), _hay.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
					if (_hay.find(_needle) == std::string::npos) continue;
				}
				_byCategory[_desc->category].push_back(_desc);
			}
			// The engine's categories first, then the classes (components and actors).
			static const char* FIRST[] = { "Events", "Flow", "Variables", "Actor", "Game", "Input", "Effects", "Math", "Logic", "Vector", "String" };
			std::vector<std::string> _order;
			for (const char* _c : FIRST) if (_byCategory.contains(_c)) _order.push_back(_c);
			for (const auto& [_c, _list] : _byCategory) if (std::find(_order.begin(), _order.end(), _c) == _order.end()) _order.push_back(_c);

			for (const std::string& _category : _order)
			{
				if (!_needle.empty() || _fromPin) ImGui::SetNextItemOpen(true, ImGuiCond_Always);
				else ImGui::SetNextItemOpen(_category == "Events" || _category == "Flow", ImGuiCond_Appearing);
				if (!ImGui::TreeNodeEx(_category.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) continue;
				for (const NodeDesc* _desc : _byCategory[_category])
				{
					ImGui::PushID(_desc->id.c_str());
					if (ImGui::Selectable(_desc->title.c_str())) _picked = _desc;
					if (ImGui::IsItemHovered() && !_desc->tooltip.empty()) ImGui::SetTooltip("%s", _desc->tooltip.c_str());
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
			if (_byCategory.empty()) ImGui::TextDisabled("Nothing found");
		}
		ImGui::EndChild();

		if (_picked)
		{
			Change();
			const int _id = AddNode(*_picked, g_paletteAt);
			// The dropped wire goes to the first pin of the new node that takes it.
			if (_fromPin)
			{
				const auto& _pins = g_paletteFrom.output ? _picked->inputs : _picked->outputs;
				for (const PinDesc& _pin : _pins)
				{
					const bool _ok = g_paletteFrom.output ? BlueprintValues::CanConnect(_fromType, _pin.type) : BlueprintValues::CanConnect(_pin.type, _fromType);
					if (!_ok) continue;
					g_lastChange.clear();
					Connect(g_paletteFrom, { _id, _pin.name, !g_paletteFrom.output });
					break;
				}
			}
			g_selected = { _id };
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// ---- The canvas

	void DrawCanvas(EditorContext& _context)
	{
		ImGuiIO& _io = ImGui::GetIO();
		ImDrawList* _draw = ImGui::GetWindowDrawList();
		const ImVec2 _origin = ImGui::GetCursorScreenPos();
		const ImVec2 _size = ImGui::GetContentRegionAvail();
		const ImVec2 _mouse = _io.MousePos;
		const bool _inside = _mouse.x >= _origin.x && _mouse.y >= _origin.y && _mouse.x <= _origin.x + _size.x && _mouse.y <= _origin.y + _size.y;

		const auto _toScreen = [&](const ImVec2& _p) { return _origin + g_pan + _p * g_zoom; };
		const auto _toGraph = [&](const ImVec2& _s) { return (_s - _origin - g_pan) * (1.f / g_zoom); };

		// Background and grid
		_draw->AddRectFilled(_origin, _origin + _size, IM_COL32(24, 25, 28, 255));
		for (const float _stepBase : { 16.f, 128.f })
		{
			const float _step = _stepBase * g_zoom;
			if (_step < 6.f) continue;
			const ImU32 _color = _stepBase > 100.f ? IM_COL32(0, 0, 0, 120) : IM_COL32(255, 255, 255, 10);
			for (float _x = std::fmod(g_pan.x, _step); _x < _size.x; _x += _step) _draw->AddLine(ImVec2(_origin.x + _x, _origin.y), ImVec2(_origin.x + _x, _origin.y + _size.y), _color);
			for (float _y = std::fmod(g_pan.y, _step); _y < _size.y; _y += _step) _draw->AddLine(ImVec2(_origin.x, _origin.y + _y), ImVec2(_origin.x + _size.x, _origin.y + _y), _color);
		}

		// Which inputs have a wire (no widget for them).
		std::set<std::pair<int, std::string>> _wired;
		for (const BlueprintLink& _link : g_asset.links) _wired.insert({ _link.toNode, _link.toPin });

		// ---- Layouts
		ImFont* _font = ImGui::GetFont();
		const float _fontSize = ImGui::GetFontSize() * g_zoom;
		const float _baseFont = ImGui::GetFontSize();
		std::vector<Layout> _layouts;
		_layouts.reserve(g_asset.nodes.size());
		for (const BlueprintNode& _node : g_asset.nodes)
		{
			Layout _layout;
			_layout.id = _node.id;
			_layout.desc = BlueprintNodes::Find(_node.type, &g_asset);
			float _width = MIN_WIDTH;
			size_t _rows = 1;
			if (_layout.desc)
			{
				_width = std::max(_width, _font->CalcTextSizeA(_baseFont, FLT_MAX, 0.f, _layout.desc->title.c_str()).x + PADDING * 3.f);
				const size_t _count = std::max(_layout.desc->inputs.size(), _layout.desc->outputs.size());
				_rows = std::max<size_t>(1, _count);
				for (size_t _r = 0; _r < _count; ++_r)
				{
					float _left = 0.f, _right = 0.f;
					if (_r < _layout.desc->inputs.size())
					{
						const PinDesc& _pin = _layout.desc->inputs[_r];
						_left = PADDING * 2.f + (_pin.type == PinType::Exec && _pin.name == "In" ? 0.f : _font->CalcTextSizeA(_baseFont, FLT_MAX, 0.f, _pin.name.c_str()).x + 6.f);
						if (_pin.type != PinType::Exec && !_wired.contains({ _node.id, _pin.name })) _left += WidgetWidth(_pin) + 6.f;
					}
					if (_r < _layout.desc->outputs.size())
					{
						const PinDesc& _pin = _layout.desc->outputs[_r];
						_right = PADDING * 2.f + (_pin.type == PinType::Exec && _pin.name == "Then" ? 0.f : _font->CalcTextSizeA(_baseFont, FLT_MAX, 0.f, _pin.name.c_str()).x);
					}
					_width = std::max(_width, _left + _right + 16.f);
				}
			}
			const float _height = (_layout.desc && _layout.desc->pure ? 6.f : HEADER) + ROW * static_cast<float>(_rows) + 6.f;
			_layout.min = _toScreen(ImVec2(_node.position.x, _node.position.y));
			_layout.max = _layout.min + ImVec2(_width, _height) * g_zoom;
			if (_layout.desc)
			{
				const float _top = (_layout.desc->pure ? 6.f : HEADER) + ROW * 0.5f;
				for (size_t _i = 0; _i < _layout.desc->inputs.size(); ++_i)
					_layout.inputs.push_back(_layout.min + ImVec2(PADDING, _top + ROW * static_cast<float>(_i)) * g_zoom);
				for (size_t _i = 0; _i < _layout.desc->outputs.size(); ++_i)
					_layout.outputs.push_back(ImVec2(_layout.max.x - PADDING * g_zoom, _layout.min.y + (_top + ROW * static_cast<float>(_i)) * g_zoom));
			}
			_layouts.push_back(std::move(_layout));
		}
		const auto _layoutOf = [&](const int _id) -> const Layout* { for (const Layout& _l : _layouts) if (_l.id == _id) return &_l; return nullptr; };

		// Pin under a point.
		const auto _pinAt = [&](const ImVec2& _p) -> PinRef
		{
			const float _reach = std::max(7.f, (PIN_RADIUS + 4.f) * g_zoom);
			for (auto _it = _layouts.rbegin(); _it != _layouts.rend(); ++_it)
			{
				if (!_it->desc) continue;
				for (size_t _i = 0; _i < _it->inputs.size(); ++_i)
					if (std::abs(_p.x - _it->inputs[_i].x) <= _reach && std::abs(_p.y - _it->inputs[_i].y) <= _reach) return { _it->id, _it->desc->inputs[_i].name, false };
				for (size_t _i = 0; _i < _it->outputs.size(); ++_i)
					if (std::abs(_p.x - _it->outputs[_i].x) <= _reach && std::abs(_p.y - _it->outputs[_i].y) <= _reach) return { _it->id, _it->desc->outputs[_i].name, true };
			}
			return PinRef();
		};
		const auto _nodeAt = [&](const ImVec2& _p) -> int
		{
			for (auto _it = _layouts.rbegin(); _it != _layouts.rend(); ++_it)
				if (_p.x >= _it->min.x && _p.y >= _it->min.y && _p.x <= _it->max.x && _p.y <= _it->max.y) return _it->id;
			return -1;
		};
		const auto _pinPosition = [&](const PinRef& _ref) -> ImVec2
		{
			const Layout* _l = _layoutOf(_ref.node);
			if (!_l || !_l->desc) return ImVec2(-1e6f, -1e6f);
			const int _index = _ref.output ? _l->desc->OutputIndex(_ref.pin) : _l->desc->InputIndex(_ref.pin);
			if (_index < 0) return ImVec2(-1e6f, -1e6f);
			return _ref.output ? _l->outputs[_index] : _l->inputs[_index];
		};
		const auto _curve = [&](const ImVec2& _a, const ImVec2& _b, const ImU32 _color, const float _thickness)
		{
			const float _bend = std::max(40.f * g_zoom, std::abs(_b.x - _a.x) * 0.5f);
			_draw->AddBezierCubic(_a, _a + ImVec2(_bend, 0.f), _b - ImVec2(_bend, 0.f), _b, _color, _thickness);
		};

		// Live: the selected actor runs this blueprint.
		const BlueprintInstance* _live = nullptr;
		if (_context.playing && _context.selected)
			if (const BlueprintComponent* _component = _context.selected->GetComponent<BlueprintComponent>())
				if (_component->blueprint == g_name && _component->IsRunning()) _live = &_component->GetInstance();

		_draw->PushClipRect(_origin, _origin + _size, true);

		// ---- Wires
		for (const BlueprintLink& _link : g_asset.links)
		{
			const ImVec2 _a = _pinPosition({ _link.fromNode, _link.fromPin, true }), _b = _pinPosition({ _link.toNode, _link.toPin, false });
			if (_a.x < -1e5f || _b.x < -1e5f) continue;
			const PinDesc* _pin = PinOf({ _link.fromNode, _link.fromPin, true });
			ImU32 _color = _pin ? PinColor(_pin->type) : IM_COL32(200, 60, 60, 255);
			float _thickness = (_pin && _pin->type == PinType::Exec ? 2.5f : 2.f) * std::max(0.6f, g_zoom);
			if (_live && _pin && _pin->type == PinType::Exec && _live->SinceRun(_link.toNode) < 0.25f)
			{
				_color = IM_COL32(255, 200, 60, 255);	// just ran
				_thickness *= 1.8f;
			}
			_curve(_a, _b, _color, _thickness);
		}
		if (g_wire.Valid())
		{
			const PinDesc* _pin = PinOf(g_wire);
			const ImVec2 _start = _pinPosition(g_wire);
			if (g_wire.output) _curve(_start, _mouse, _pin ? PinColor(_pin->type) : IM_COL32_WHITE, 2.f);
			else _curve(_mouse, _start, _pin ? PinColor(_pin->type) : IM_COL32_WHITE, 2.f);
		}

		// ---- Nodes (with the widgets of their inputs: real ImGui items)
		ImGui::SetWindowFontScale(g_zoom);
		const bool _widgets = g_zoom >= 0.55f;
		const ImU32 _accent = EditorTheme::GetU32(EditorTheme::Custom::Selection);
		for (size_t _n = 0; _n < g_asset.nodes.size(); ++_n)
		{
			BlueprintNode& _node = g_asset.nodes[_n];
			const Layout& _l = _layouts[_n];
			if (_l.max.x < _origin.x || _l.max.y < _origin.y || _l.min.x > _origin.x + _size.x || _l.min.y > _origin.y + _size.y) continue;	// off the canvas
			const float _rounding = 6.f * g_zoom;
			const bool _selected = g_selected.contains(_node.id);
			const bool _ran = _live && _live->SinceRun(_node.id) < 0.25f;

			_draw->AddRectFilled(_l.min + ImVec2(3.f, 4.f) * g_zoom, _l.max + ImVec2(3.f, 4.f) * g_zoom, IM_COL32(0, 0, 0, 90), _rounding);	// shadow
			_draw->AddRectFilled(_l.min, _l.max, IM_COL32(38, 40, 45, 240), _rounding);
			if (!_l.desc)
			{
				_draw->AddRect(_l.min, _l.max, IM_COL32(220, 60, 60, 255), _rounding, 0, 2.f);
				_draw->AddText(_font, _fontSize, _l.min + ImVec2(PADDING, 6.f) * g_zoom, IM_COL32(255, 120, 120, 255), ("Unknown: " + _node.type).c_str());
				if (_selected) _draw->AddRect(_l.min, _l.max, _accent, _rounding, 0, 2.5f);
				continue;
			}
			if (!_l.desc->pure)
			{
				_draw->AddRectFilled(_l.min, ImVec2(_l.max.x, _l.min.y + HEADER * g_zoom), HeaderColor(_l.desc), _rounding, ImDrawFlags_RoundCornersTop);
				_draw->AddText(_font, _fontSize, _l.min + ImVec2(PADDING, (HEADER - _baseFont) * 0.5f) * g_zoom, IM_COL32(245, 245, 245, 255), _l.desc->title.c_str());
			}
			else	// a value node: its title, small, in the corner
				_draw->AddText(_font, _fontSize * 0.8f, ImVec2(_l.min.x + PADDING * g_zoom, _l.min.y - _fontSize * 0.95f), IM_COL32(170, 175, 185, 255), _l.desc->title.c_str());

			const ImU32 _border = _ran ? IM_COL32(255, 200, 60, 255) : (_selected ? _accent : IM_COL32(12, 12, 14, 255));
			_draw->AddRect(_l.min, _l.max, _border, _rounding, 0, _ran || _selected ? 2.5f : 1.f);

			// Inputs
			for (size_t _i = 0; _i < _l.desc->inputs.size(); ++_i)
			{
				const PinDesc& _pin = _l.desc->inputs[_i];
				const ImVec2 _p = _l.inputs[_i];
				const bool _hasWire = _wired.contains({ _node.id, _pin.name });
				if (_pin.type == PinType::Exec)
				{
					const float _s = 6.f * g_zoom;
					const ImVec2 _a(_p.x - _s * 0.6f, _p.y - _s), _b(_p.x - _s * 0.6f, _p.y + _s), _c(_p.x + _s, _p.y);
					if (_hasWire) _draw->AddTriangleFilled(_a, _b, _c, PinColor(_pin.type));
					else _draw->AddTriangle(_a, _b, _c, PinColor(_pin.type), 1.5f);
				}
				else if (_hasWire) _draw->AddCircleFilled(_p, PIN_RADIUS * g_zoom, PinColor(_pin.type));
				else _draw->AddCircle(_p, PIN_RADIUS * g_zoom, PinColor(_pin.type), 0, 1.5f);

				float _x = _p.x + (PADDING * 0.8f) * g_zoom;
				if (!(_pin.type == PinType::Exec && _pin.name == "In"))
				{
					_draw->AddText(_font, _fontSize, ImVec2(_x, _p.y - _fontSize * 0.5f), IM_COL32(215, 218, 225, 255), _pin.name.c_str());
					_x += _font->CalcTextSizeA(_fontSize, FLT_MAX, 0.f, _pin.name.c_str()).x + 6.f * g_zoom;
				}
				if (_pin.type != PinType::Exec && !_hasWire)
				{
					if (_widgets)
					{
						ImGui::SetCursorScreenPos(ImVec2(_x, _p.y - ImGui::GetFrameHeight() * 0.5f));
						DrawValueWidget(_node, _pin, WidgetWidth(_pin) * g_zoom);
					}
					else
						_draw->AddText(_font, _fontSize, ImVec2(_x, _p.y - _fontSize * 0.5f), IM_COL32(150, 150, 160, 255), ValueOf(_node, _pin).c_str());
				}
			}
			// Outputs
			for (size_t _i = 0; _i < _l.desc->outputs.size(); ++_i)
			{
				const PinDesc& _pin = _l.desc->outputs[_i];
				const ImVec2 _p = _l.outputs[_i];
				const bool _hasWire = std::any_of(g_asset.links.begin(), g_asset.links.end(), [&](const BlueprintLink& _k) { return _k.fromNode == _node.id && _k.fromPin == _pin.name; });
				if (_pin.type == PinType::Exec)
				{
					const float _s = 6.f * g_zoom;
					const ImVec2 _a(_p.x - _s * 0.6f, _p.y - _s), _b(_p.x - _s * 0.6f, _p.y + _s), _c(_p.x + _s, _p.y);
					if (_hasWire) _draw->AddTriangleFilled(_a, _b, _c, PinColor(_pin.type));
					else _draw->AddTriangle(_a, _b, _c, PinColor(_pin.type), 1.5f);
				}
				else if (_hasWire) _draw->AddCircleFilled(_p, PIN_RADIUS * g_zoom, PinColor(_pin.type));
				else _draw->AddCircle(_p, PIN_RADIUS * g_zoom, PinColor(_pin.type), 0, 1.5f);
				if (!(_pin.type == PinType::Exec && _pin.name == "Then"))
				{
					const float _w = _font->CalcTextSizeA(_fontSize, FLT_MAX, 0.f, _pin.name.c_str()).x;
					_draw->AddText(_font, _fontSize, ImVec2(_p.x - PADDING * 0.8f * g_zoom - _w, _p.y - _fontSize * 0.5f), IM_COL32(215, 218, 225, 255), _pin.name.c_str());
				}
			}
		}
		ImGui::SetWindowFontScale(1.f);

		// Selection box
		if (g_boxing)
		{
			_draw->AddRectFilled(g_boxStart, _mouse, IM_COL32(76, 141, 246, 40));
			_draw->AddRect(g_boxStart, _mouse, IM_COL32(76, 141, 246, 200));
		}
		if (g_asset.nodes.empty())
			_draw->AddText(_origin + ImVec2(20.f, 20.f), IM_COL32(200, 200, 205, 200), "Right click to add a node (Event BeginPlay, Print, Branch...).");
		_draw->AddText(ImVec2(_origin.x + 10.f, _origin.y + _size.y - ImGui::GetFontSize() - 8.f), IM_COL32(200, 200, 205, 110),
			"Right click: add a node   Drag a pin: wire   Alt+click a pin: cut   Delete   Ctrl+D copy   Ctrl+Z / Y   Ctrl+S save   Right mouse: move   Wheel: zoom");
		_draw->PopClipRect();

		// ---- Mouse (on the canvas, not on a widget of a node)
		const bool _canvasHovered = ImGui::IsWindowHovered() && _inside && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive();
		if (_canvasHovered && _io.MouseWheel != 0.f)
		{
			const ImVec2 _before = _toGraph(_mouse);
			g_zoom = std::clamp(g_zoom * (_io.MouseWheel > 0.f ? 1.1f : 1.f / 1.1f), 0.3f, 2.f);
			g_pan = _mouse - _origin - _before * g_zoom;
		}
		if (ImGui::IsWindowHovered() && (ImGui::IsMouseDragging(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Middle)))
			g_pan = g_pan + _io.MouseDelta;

		if (_canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			const PinRef _pin = _pinAt(_mouse);
			const int _node = _nodeAt(_mouse);
			if (_pin.Valid())
			{
				if (_io.KeyAlt) BreakLinks(_pin);
				else g_wire = _pin;
			}
			else if (_node >= 0)
			{
				if (_io.KeyCtrl) { if (!g_selected.erase(_node)) g_selected.insert(_node); }
				else if (!g_selected.contains(_node)) g_selected = { _node };
				g_dragging = true;
				g_dragMoved = false;
			}
			else
			{
				if (!_io.KeyCtrl) g_selected.clear();
				g_boxing = true;
				g_boxStart = _mouse;
			}
		}

		if (g_dragging && ImGui::IsMouseDown(ImGuiMouseButton_Left) && (_io.MouseDelta.x != 0.f || _io.MouseDelta.y != 0.f))
		{
			if (!g_dragMoved) Change("move");
			g_dragMoved = true;
			for (BlueprintNode& _node : g_asset.nodes)
				if (g_selected.contains(_node.id))
					_node.position = FVector2(_node.position.x + _io.MouseDelta.x / g_zoom, _node.position.y + _io.MouseDelta.y / g_zoom);
		}

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			if (g_wire.Valid())
			{
				const PinRef _target = _pinAt(_mouse);
				if (_target.Valid()) Connect(g_wire, _target);
				else if (_inside && _nodeAt(_mouse) < 0)
				{
					// Dropped on nothing: the list of nodes that can take this wire.
					g_paletteFrom = g_wire;
					g_paletteAt = _toGraph(_mouse);
					g_search[0] = '\0';
					g_focusSearch = true;
					ImGui::OpenPopup("##palette");
				}
				g_wire = PinRef();
			}
			if (g_boxing)
			{
				const ImVec2 _a(std::min(g_boxStart.x, _mouse.x), std::min(g_boxStart.y, _mouse.y)), _b(std::max(g_boxStart.x, _mouse.x), std::max(g_boxStart.y, _mouse.y));
				for (const Layout& _l : _layouts)
					if (_l.max.x >= _a.x && _l.min.x <= _b.x && _l.max.y >= _a.y && _l.min.y <= _b.y) g_selected.insert(_l.id);
				g_boxing = false;
			}
			if (g_dragging)
				for (BlueprintNode& _node : g_asset.nodes)	// whole units: the file stays readable
					if (g_selected.contains(_node.id)) _node.position = FVector2(std::round(_node.position.x), std::round(_node.position.y));
			g_dragging = false;
		}
		if (!ImGui::IsAnyMouseDown()) g_lastChange.clear();	// the next change is a new undo step

		// Right click without moving the view: the list of nodes.
		if (_canvasHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		{
			const ImVec2 _drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
			if (_drag.x * _drag.x + _drag.y * _drag.y < 16.f)
			{
				g_paletteFrom = PinRef();
				g_paletteAt = _toGraph(_mouse);
				g_search[0] = '\0';
				g_focusSearch = true;
				ImGui::OpenPopup("##palette");
			}
		}
		DrawPalette();

		// Keys
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !_io.WantTextInput)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Delete)) DeleteSelected();
			if (_io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D)) DuplicateSelected();
			if (_io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) for (const BlueprintNode& _node : g_asset.nodes) g_selected.insert(_node.id);
		}
	}

	void Undo()
	{
		if (g_undo.empty()) return;
		g_redo.push_back(g_asset);
		g_asset = g_undo.back();
		g_undo.pop_back();
		g_dirty = true;
		g_selected.clear();
		g_lastChange.clear();
	}

	void Redo()
	{
		if (g_redo.empty()) return;
		g_undo.push_back(g_asset);
		g_asset = g_redo.back();
		g_redo.pop_back();
		g_dirty = true;
		g_selected.clear();
		g_lastChange.clear();
	}

	// No blueprint open: the list of blueprints and a button for a new one.
	void DrawChooser()
	{
		ImGui::TextWrapped("Blueprints are visual scripts: put a BlueprintComponent on an actor and pick one to run it.");
		ImGui::Spacing();
		ImGui::SetNextItemWidth(220.f);
		ImGui::InputText("##newName", g_newName, sizeof(g_newName));
		ImGui::SameLine();
		if (ImGui::Button("+ New Blueprint"))
		{
			const std::string _created = CreateBlueprint(g_newName);
			if (!_created.empty()) Open(_created);
		}
		ImGui::SeparatorText("Content/Blueprints");
		const std::vector<std::string> _names = BlueprintAsset::List();
		for (const std::string& _name : _names)
			if (ImGui::Selectable(_name.c_str())) Open(_name);
		if (_names.empty()) ImGui::TextDisabled("None yet.");
	}
}

namespace Krampus
{
	void OpenBlueprintEditor(const std::string& _name)
	{
		g_open = true;
		if (!_name.empty() && _name != g_name) Open(_name);
		ImGui::SetWindowFocus("Blueprint Editor");
	}

	std::string CreateBlueprintAsset(const std::string& _name)
	{
		return CreateBlueprint(_name);
	}

	void SaveBlueprintEditorChanges()
	{
		if (g_dirty) SaveNow();
	}

	void DrawBlueprintEditor(EditorContext& _context)
	{
		if (!g_open) return;
		ImGui::SetNextWindowSize(ImVec2(1200.f, 700.f), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Blueprint Editor", &g_open, ImGuiWindowFlags_MenuBar))
		{
			ImGui::End();
			return;
		}
		if (!g_open && g_dirty) SaveNow();	// closed: nothing is lost

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Blueprint"))
			{
				if (ImGui::MenuItem("Save", "Ctrl+S", false, !g_name.empty())) SaveNow();
				if (ImGui::BeginMenu("Open"))
				{
					for (const std::string& _name : BlueprintAsset::List())
						if (ImGui::MenuItem(_name.c_str(), nullptr, _name == g_name)) Open(_name);
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Close", nullptr, false, !g_name.empty()))
				{
					if (g_dirty) SaveNow();
					g_name.clear();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit", !g_name.empty()))
			{
				if (ImGui::MenuItem("Undo", "Ctrl+Z", false, !g_undo.empty())) Undo();
				if (ImGui::MenuItem("Redo", "Ctrl+Y", false, !g_redo.empty())) Redo();
				ImGui::Separator();
				if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, !g_selected.empty())) DuplicateSelected();
				if (ImGui::MenuItem("Delete", "Delete", false, !g_selected.empty())) DeleteSelected();
				ImGui::EndMenu();
			}
			if (!g_name.empty())
			{
				ImGui::TextDisabled("|  %s%s", g_name.c_str(), g_dirty ? " *" : "");
				if (_context.playing && g_dirty) ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::ConsoleWarning), "  changes run at the next Play");
			}
			ImGui::EndMenuBar();
		}

		if (g_name.empty() || !BlueprintAsset::Exists(g_name))
		{
			g_name.clear();
			DrawChooser();
			ImGui::End();
			return;
		}

		// Shortcuts of the window
		const ImGuiIO& _io = ImGui::GetIO();
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !_io.WantTextInput && _io.KeyCtrl)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_S)) SaveNow();
			if (ImGui::IsKeyPressed(ImGuiKey_Z)) { if (_io.KeyShift) Redo(); else Undo(); }
			if (ImGui::IsKeyPressed(ImGuiKey_Y)) Redo();
		}

		const float _side = 300.f * ImGui::GetFontSize() / 16.f;
		if (ImGui::BeginChild("##blueprintSide", ImVec2(_side, 0.f), ImGuiChildFlags_Borders))
			DrawSidePanel(_context);
		ImGui::EndChild();
		ImGui::SameLine();
		if (ImGui::BeginChild("##blueprintCanvas", ImVec2(0.f, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove))
			DrawCanvas(_context);
		ImGui::EndChild();
		ImGui::End();
	}
}
