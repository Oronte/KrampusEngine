#include "EditorContext.h"
#include "EditorTheme.h"
#include "Actors/Actor.h"
#include "Components/AnimationComponentSM.h"
#include <imgui.h>
#include <cmath>
#include <format>

using namespace Krampus;

// The Animation Graph: the state machine of an AnimationComponentSM drawn as boxes (its animations) and arrows (transitions).
//   - drag from the dot on the right of a box to another box: a transition;
//   - click an arrow: its conditions on the right (all must be true), on the parameters the game sets (SetBool, SetTrigger...);
//   - the Any box: transitions that leave whatever animation plays (hurt, die);
//   - drag a box to move it, the right or middle mouse button to move the view, the wheel to zoom.
// In Play the animation that plays is lit and the parameters can be changed live; the graph itself is edited while stopped.

namespace
{
	constexpr float		NODE_WIDTH		= 150.f;
	constexpr float		NODE_HEIGHT		= 46.f;
	constexpr float		HANDLE_RADIUS	= 6.f;
	constexpr float		PANEL_WIDTH		= 330.f;
	const std::string	ANY				= AnimationTransition::ANY;

	bool				g_open			= false;
	const void*			g_component		= nullptr;	// the component the state below is about (compared only, never used)
	ImVec2				g_pan			= ImVec2(40.f, 40.f);
	float				g_zoom			= 1.f;
	std::string			g_selectedNode;
	int					g_selectedTransition = -1;
	std::string			g_dragNode;
	ImVec2				g_dragGrab;					// where in the box it was grabbed (graph units)
	bool				g_dragMoved		= false;
	std::string			g_linkFrom;					// a transition being drawn from this box
	std::string			g_menuNode;					// the box the right click menu is about
	char				g_renameBuffer[64]	= "";
	int					g_renaming		= -1;

	const char* TYPE_NAMES[] = { "Bool", "Int", "Float", "Trigger" };
	const char* TEST_NAMES[] = { "is true", "is false", "==", "!=", ">", "<" };

	bool IsTruthType(const int _type)
	{
		return _type == static_cast<int>(AnimationParameterType::Bool) || _type == static_cast<int>(AnimationParameterType::Trigger);
	}

	ImVec2 operator+(const ImVec2& _a, const ImVec2& _b) { return ImVec2(_a.x + _b.x, _a.y + _b.y); }
	ImVec2 operator-(const ImVec2& _a, const ImVec2& _b) { return ImVec2(_a.x - _b.x, _a.y - _b.y); }
	ImVec2 operator*(const ImVec2& _a, const float _f) { return ImVec2(_a.x * _f, _a.y * _f); }

	float DistanceToSegment(const ImVec2& _p, const ImVec2& _a, const ImVec2& _b)
	{
		const ImVec2 _ab = _b - _a, _ap = _p - _a;
		const float _length = _ab.x * _ab.x + _ab.y * _ab.y;
		const float _t = _length > 0.f ? std::clamp((_ap.x * _ab.x + _ap.y * _ab.y) / _length, 0.f, 1.f) : 0.f;
		const ImVec2 _closest = _a + _ab * _t;
		return std::sqrt((_p.x - _closest.x) * (_p.x - _closest.x) + (_p.y - _closest.y) * (_p.y - _closest.y));
	}

	// Where a line leaving the center of a box along _direction crosses its edge.
	ImVec2 EdgeOfBox(const ImVec2& _center, const ImVec2& _half, const ImVec2& _direction)
	{
		const float _tx = std::abs(_direction.x) > 1e-5f ? _half.x / std::abs(_direction.x) : 1e9f;
		const float _ty = std::abs(_direction.y) > 1e-5f ? _half.y / std::abs(_direction.y) : 1e9f;
		return _center + _direction * std::min(_tx, _ty);
	}

	// Everything the window needs about the selected actor, found again every frame.
	struct Graph
	{
		AnimationComponentSM*		component	= nullptr;
		std::vector<std::string>	names;		// the animations (clips), then "*"
		std::string					owner;
	};

	bool FindGraph(EditorContext& _context, Graph& _graph)
	{
		if (!_context.selected) return false;
		_graph.component = _context.selected->GetComponent<AnimationComponentSM>();
		if (!_graph.component) return false;
		_graph.owner = _context.selected->GetName().StdString();
		for (const AnimationClip& _clip : _graph.component->clips)
		{
			const std::string _name = _clip.name.StdString();
			if (std::find(_graph.names.begin(), _graph.names.end(), _name) == _graph.names.end()) _graph.names.push_back(_name);
		}
		_graph.names.push_back(ANY);
		return true;
	}

	// Where a box is (graph units): the saved place, else a grid (the Any box on the left).
	ImVec2 PositionOf(const AnimationGraph& _graph, const std::vector<std::string>& _names, const std::string& _name)
	{
		const auto _saved = _graph.positions.find(_name);
		if (_saved != _graph.positions.end()) return ImVec2(_saved->second.x, _saved->second.y);
		if (_name == ANY) return ImVec2(0.f, 0.f);
		const int _index = static_cast<int>(std::find(_names.begin(), _names.end(), _name) - _names.begin());
		return ImVec2(220.f + (_index % 3) * 210.f, (_index / 3) * 110.f);
	}

	bool HasNode(const Graph& _graph, const std::string& _name)
	{
		return std::find(_graph.names.begin(), _graph.names.end(), _name) != _graph.names.end();
	}

	void Record(EditorContext& _context, const std::string& _label, const std::string& _key = "")
	{
		_context.history.Record(_context, _label, _key.empty() ? std::string() : "graph." + _key);
	}

	// ---- Right panel: parameters

	void DrawParameters(EditorContext& _context, Graph& _graph, const bool _playing)
	{
		AnimationGraph& _data = _graph.component->graph;
		ImGui::SeparatorText("Parameters");
		ImGui::TextDisabled(_playing ? "Live values: change them to test the transitions." : "Values the game sets: SetBool, SetInt, SetFloat, SetTrigger.");

		int _remove = -1;
		for (int _i = 0; _i < static_cast<int>(_data.parameters.size()); ++_i)
		{
			AnimationParameter& _parameter = _data.parameters[_i];
			ImGui::PushID(_i);

			// Name: applied when the field is left (one undo step, and the conditions follow the new name).
			ImGui::SetNextItemWidth(110.f);
			if (g_renaming != _i) std::snprintf(g_renameBuffer, sizeof(g_renameBuffer), "%s", _parameter.name.c_str());
			ImGui::BeginDisabled(_playing);
			char _buffer[64];
			std::snprintf(_buffer, sizeof(_buffer), "%s", g_renaming == _i ? g_renameBuffer : _parameter.name.c_str());
			if (ImGui::InputText("##name", _buffer, sizeof(_buffer))) { g_renaming = _i; std::snprintf(g_renameBuffer, sizeof(g_renameBuffer), "%s", _buffer); }
			if (ImGui::IsItemDeactivatedAfterEdit() && g_renaming == _i)
			{
				const std::string _new = g_renameBuffer;
				const bool _taken = _data.FindParameter(_new) != nullptr;
				if (!_new.empty() && !_taken && _new != _parameter.name)
				{
					Record(_context, "Rename parameter");
					for (AnimationTransition& _transition : _data.transitions)
						for (AnimationCondition& _condition : _transition.conditions)
							if (_condition.parameter == _parameter.name) _condition.parameter = _new;
					_data.parameters[_i].name = _new;
					_graph.component->ResetParameters();
				}
				else if (_taken && _new != _parameter.name) LOG_WARNING("Animation Graph: a parameter is already named \"" + _new + "\"");
				g_renaming = -1;
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(72.f);
			int _type = _parameter.type;
			if (ImGui::Combo("##type", &_type, TYPE_NAMES, 4) && _type != _parameter.type)
			{
				Record(_context, "Change parameter type");
				_data.parameters[_i].type = _type;
				if (IsTruthType(_type)) _data.parameters[_i].value = _data.parameters[_i].value != 0.f ? 1.f : 0.f;
				for (AnimationTransition& _transition : _data.transitions)	// the tests must suit the new type
					for (AnimationCondition& _condition : _transition.conditions)
						if (_condition.parameter == _data.parameters[_i].name)
						{
							if (IsTruthType(_type) && _condition.test > 1) _condition.test = 0;
							if (!IsTruthType(_type) && _condition.test < 2) _condition.test = 4;
						}
				_graph.component->ResetParameters();
			}
			ImGui::EndDisabled();

			// Value: the starting one while editing, the live one in Play.
			ImGui::SameLine();
			ImGui::SetNextItemWidth(70.f);
			AnimationParameter& _p = _data.parameters[_i];
			const float _shown = _playing ? _graph.component->GetParameter(_p.name) : _p.value;
			if (IsTruthType(_p.type))
			{
				bool _on = _shown != 0.f;
				if (ImGui::Checkbox("##value", &_on))
				{
					if (_playing) _graph.component->SetBool(_p.name, _on);
					else { Record(_context, "Change parameter value"); _p.value = _on ? 1.f : 0.f; _graph.component->ResetParameters(); }
				}
			}
			else if (_p.type == static_cast<int>(AnimationParameterType::Int))
			{
				int _value = static_cast<int>(std::round(_shown));
				if (ImGui::DragInt("##value", &_value))
				{
					if (_playing) _graph.component->SetInt(_p.name, _value);
					else { Record(_context, "Change parameter value", "value." + _p.name); _p.value = static_cast<float>(_value); _graph.component->ResetParameters(); }
				}
			}
			else
			{
				float _value = _shown;
				if (ImGui::DragFloat("##value", &_value, 0.05f))
				{
					if (_playing) _graph.component->SetFloat(_p.name, _value);
					else { Record(_context, "Change parameter value", "value." + _p.name); _p.value = _value; _graph.component->ResetParameters(); }
				}
			}

			ImGui::SameLine();
			ImGui::BeginDisabled(_playing);
			if (ImGui::SmallButton("x")) _remove = _i;
			ImGui::EndDisabled();
			ImGui::PopID();
		}

		if (_remove >= 0)
		{
			Record(_context, "Delete parameter");
			const std::string _gone = _data.parameters[_remove].name;
			_data.parameters.erase(_data.parameters.begin() + _remove);
			for (AnimationTransition& _transition : _data.transitions)
				std::erase_if(_transition.conditions, [&](const AnimationCondition& _condition) { return _condition.parameter == _gone; });
			_graph.component->ResetParameters();
			g_renaming = -1;
		}

		ImGui::BeginDisabled(_playing);
		if (ImGui::Button("+ Parameter", ImVec2(-1.f, 0.f)))
		{
			Record(_context, "Add parameter");
			std::string _name = "param";
			for (int _n = 1; _data.FindParameter(_name); ++_n) _name = "param" + std::to_string(_n);
			_data.parameters.push_back({ _name, static_cast<int>(AnimationParameterType::Bool), 0.f });
			_graph.component->ResetParameters();
		}
		ImGui::EndDisabled();
	}

	// ---- Right panel: the selected transition

	void DrawTransition(EditorContext& _context, Graph& _graph, const bool _playing)
	{
		AnimationGraph& _data = _graph.component->graph;
		AnimationTransition& _transition = _data.transitions[g_selectedTransition];
		ImGui::SeparatorText("Transition");
		ImGui::Text("%s  ->  %s", _transition.from == ANY ? "Any animation" : _transition.from.c_str(), _transition.to.c_str());

		ImGui::BeginDisabled(_playing);
		bool _wait = _transition.waitForEnd;
		if (ImGui::Checkbox("Wait for the end of the animation", &_wait))
		{
			Record(_context, "Edit transition");
			_data.transitions[g_selectedTransition].waitForEnd = _wait;
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Only once the current animation reached its last frame (an attack plays to its end).");

		ImGui::TextDisabled(_transition.conditions.empty() ? "No condition: taken at once." : "All of these must be true:");
		int _remove = -1;
		for (int _c = 0; _c < static_cast<int>(_transition.conditions.size()); ++_c)
		{
			AnimationCondition& _condition = _data.transitions[g_selectedTransition].conditions[_c];
			ImGui::PushID(_c);
			const AnimationParameter* _parameter = _data.FindParameter(_condition.parameter);

			ImGui::SetNextItemWidth(110.f);
			if (ImGui::BeginCombo("##parameter", _condition.parameter.empty() ? "(parameter)" : _condition.parameter.c_str()))
			{
				for (const AnimationParameter& _choice : _data.parameters)
					if (ImGui::Selectable(_choice.name.c_str(), _choice.name == _condition.parameter))
					{
						Record(_context, "Edit condition");
						_condition.parameter = _choice.name;
						if (IsTruthType(_choice.type) && _condition.test > 1) _condition.test = 0;
						if (!IsTruthType(_choice.type) && _condition.test < 2) _condition.test = 4;
					}
				if (_data.parameters.empty()) ImGui::TextDisabled("Add a parameter first");
				ImGui::EndCombo();
			}

			const bool _truth = !_parameter || IsTruthType(_parameter->type);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(_truth ? 90.f : 50.f);
			if (ImGui::BeginCombo("##test", TEST_NAMES[std::clamp(_condition.test, 0, 5)]))
			{
				for (int _t = _truth ? 0 : 2; _t < (_truth ? 2 : 6); ++_t)
					if (ImGui::Selectable(TEST_NAMES[_t], _t == _condition.test))
					{
						Record(_context, "Edit condition");
						_condition.test = _t;
					}
				ImGui::EndCombo();
			}

			if (!_truth)
			{
				ImGui::SameLine();
				ImGui::SetNextItemWidth(70.f);
				if (_parameter->type == static_cast<int>(AnimationParameterType::Int))
				{
					int _value = static_cast<int>(std::round(_condition.value));
					if (ImGui::DragInt("##value", &_value)) { Record(_context, "Edit condition", "condition"); _condition.value = static_cast<float>(_value); }
				}
				else if (ImGui::DragFloat("##value", &_condition.value, 0.05f)) Record(_context, "Edit condition", "condition");
			}
			if (!_condition.parameter.empty() && !_parameter)
				ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::ConsoleWarning), "No parameter \"%s\"", _condition.parameter.c_str());

			ImGui::SameLine();
			if (ImGui::SmallButton("x")) _remove = _c;
			ImGui::PopID();
		}
		if (_remove >= 0)
		{
			Record(_context, "Delete condition");
			auto& _conditions = _data.transitions[g_selectedTransition].conditions;
			_conditions.erase(_conditions.begin() + _remove);
		}

		if (ImGui::Button("+ Condition", ImVec2(-1.f, 0.f)))
		{
			Record(_context, "Add condition");
			AnimationCondition _condition;
			if (!_data.parameters.empty())
			{
				_condition.parameter = _data.parameters.front().name;
				_condition.test = IsTruthType(_data.parameters.front().type) ? 0 : 4;
			}
			_data.transitions[g_selectedTransition].conditions.push_back(_condition);
		}

		// Order: the first transition (of the same animation) whose conditions are true is taken.
		ImGui::Spacing();
		if (ImGui::Button("Higher priority") && g_selectedTransition > 0)
		{
			Record(_context, "Reorder transitions");
			std::swap(_data.transitions[g_selectedTransition], _data.transitions[g_selectedTransition - 1]);
			--g_selectedTransition;
		}
		ImGui::SameLine();
		if (ImGui::Button("Lower") && g_selectedTransition + 1 < static_cast<int>(_data.transitions.size()))
		{
			Record(_context, "Reorder transitions");
			std::swap(_data.transitions[g_selectedTransition], _data.transitions[g_selectedTransition + 1]);
			++g_selectedTransition;
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete"))
		{
			Record(_context, "Delete transition");
			_data.transitions.erase(_data.transitions.begin() + g_selectedTransition);
			g_selectedTransition = -1;
		}
		ImGui::EndDisabled();
	}

	void DrawNodeDetails(EditorContext& _context, Graph& _graph, const bool _playing)
	{
		AnimationComponentSM& _component = *_graph.component;
		ImGui::SeparatorText(g_selectedNode == ANY ? "Any animation" : g_selectedNode.c_str());
		if (g_selectedNode == ANY)
		{
			ImGui::TextWrapped("Its transitions leave whatever animation plays, before the others are tested (hurt, die...).");
			return;
		}
		const bool _isStart = _component.startAnimation.StdString() == g_selectedNode
			|| (_component.startAnimation.StdString().empty() && !_graph.names.empty() && _graph.names.front() == g_selectedNode);
		ImGui::BeginDisabled(_playing || _isStart);
		if (ImGui::Button(_isStart ? "Starting animation" : "Set as starting animation", ImVec2(-1.f, 0.f)))
		{
			Record(_context, "Set starting animation");
			_component.startAnimation = String(g_selectedNode);
			_component.ApplyClips();
		}
		ImGui::EndDisabled();
		if (ImGui::Button("Edit its frames (Animation Editor)", ImVec2(-1.f, 0.f))) OpenAnimationEditor();

		ImGui::TextDisabled("Leaves to (first true wins):");
		for (int _i = 0; _i < static_cast<int>(_component.graph.transitions.size()); ++_i)
		{
			const AnimationTransition& _transition = _component.graph.transitions[_i];
			if (_transition.from != g_selectedNode) continue;
			ImGui::PushID(_i);
			if (ImGui::Selectable(("-> " + _transition.to + "  (" + std::to_string(_transition.conditions.size()) + " condition(s))").c_str(), false))
			{
				g_selectedTransition = _i;
				g_selectedNode.clear();
			}
			ImGui::PopID();
		}
	}

	// ---- The canvas

	void DrawCanvas(EditorContext& _context, Graph& _graph, const bool _playing)
	{
		AnimationComponentSM& _component = *_graph.component;
		AnimationGraph& _data = _component.graph;
		ImGuiIO& _io = ImGui::GetIO();

		const ImVec2 _origin = ImGui::GetCursorScreenPos();
		const ImVec2 _size(std::max(50.f, ImGui::GetContentRegionAvail().x), std::max(50.f, ImGui::GetContentRegionAvail().y));
		ImGui::InvisibleButton("##canvas", _size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
		const bool _hovered = ImGui::IsItemHovered();
		const bool _active = ImGui::IsItemActive();
		const ImVec2 _mouse = _io.MousePos;

		ImDrawList* _draw = ImGui::GetWindowDrawList();
		_draw->PushClipRect(_origin, _origin + _size, true);
		_draw->AddRectFilled(_origin, _origin + _size, ImGui::GetColorU32(ImGuiCol_FrameBg));

		// Grid
		const float _step = 32.f * g_zoom;
		const ImU32 _gridColor = IM_COL32(255, 255, 255, 12);
		for (float _x = std::fmod(g_pan.x, _step); _x < _size.x; _x += _step) _draw->AddLine(ImVec2(_origin.x + _x, _origin.y), ImVec2(_origin.x + _x, _origin.y + _size.y), _gridColor);
		for (float _y = std::fmod(g_pan.y, _step); _y < _size.y; _y += _step) _draw->AddLine(ImVec2(_origin.x, _origin.y + _y), ImVec2(_origin.x + _size.x, _origin.y + _y), _gridColor);

		const auto _toScreen = [&](const ImVec2& _p) { return _origin + g_pan + _p * g_zoom; };
		const auto _toGraph = [&](const ImVec2& _s) { return (_s - _origin - g_pan) * (1.f / g_zoom); };
		const ImVec2 _nodeSize = ImVec2(NODE_WIDTH, NODE_HEIGHT) * g_zoom;
		const auto _nodeMin = [&](const std::string& _name) { return _toScreen(PositionOf(_data, _graph.names, _name)); };
		const auto _nodeCenter = [&](const std::string& _name) { return _nodeMin(_name) + _nodeSize * 0.5f; };
		const auto _handleOf = [&](const std::string& _name) { return _nodeMin(_name) + ImVec2(_nodeSize.x, _nodeSize.y * 0.5f); };
		const auto _nodeAt = [&](const ImVec2& _p) -> std::string
		{
			for (auto _it = _graph.names.rbegin(); _it != _graph.names.rend(); ++_it)
			{
				const ImVec2 _min = _nodeMin(*_it);
				if (_p.x >= _min.x && _p.y >= _min.y && _p.x <= _min.x + _nodeSize.x && _p.y <= _min.y + _nodeSize.y) return *_it;
			}
			return "";
		};
		const auto _handleAt = [&](const ImVec2& _p) -> std::string
		{
			for (const std::string& _name : _graph.names)
			{
				const ImVec2 _h = _handleOf(_name);
				if ((_p.x - _h.x) * (_p.x - _h.x) + (_p.y - _h.y) * (_p.y - _h.y) <= std::pow((HANDLE_RADIUS + 3.f) * g_zoom, 2.f)) return _name;
			}
			return "";
		};

		// Arrows: the segment of each transition, moved to the side when the opposite one exists too.
		struct Arrow { ImVec2 a, b; };
		std::vector<Arrow> _arrows(_data.transitions.size());
		const ImU32 _accent = EditorTheme::GetU32(EditorTheme::Custom::Selection);
		for (size_t _i = 0; _i < _data.transitions.size(); ++_i)
		{
			const AnimationTransition& _t = _data.transitions[_i];
			if (!HasNode(_graph, _t.from) || !HasNode(_graph, _t.to) || _t.to == ANY) { _arrows[_i] = { ImVec2(-1e6f, -1e6f), ImVec2(-1e6f, -1e6f) }; continue; }
			const ImVec2 _from = _nodeCenter(_t.from), _to = _nodeCenter(_t.to);
			ImVec2 _direction = _to - _from;
			const float _length = std::sqrt(_direction.x * _direction.x + _direction.y * _direction.y);
			if (_length < 1.f) { _arrows[_i] = { _from, _to }; continue; }
			_direction = _direction * (1.f / _length);
			const ImVec2 _side(-_direction.y, _direction.x);
			const bool _opposite = std::any_of(_data.transitions.begin(), _data.transitions.end(), [&](const AnimationTransition& _o) { return _o.from == _t.to && _o.to == _t.from; });
			const ImVec2 _shift = _opposite ? _side * (7.f * g_zoom) : ImVec2(0.f, 0.f);
			const ImVec2 _half = _nodeSize * 0.5f;
			_arrows[_i] = { EdgeOfBox(_from, _half, _direction) + _shift, EdgeOfBox(_to, _half, _direction * -1.f) + _shift };
		}

		// ---- Mouse
		if (_hovered && _io.MouseWheel != 0.f)
		{
			const ImVec2 _before = _toGraph(_mouse);
			g_zoom = std::clamp(g_zoom * (_io.MouseWheel > 0.f ? 1.1f : 1.f / 1.1f), 0.35f, 2.5f);
			g_pan = _mouse - _origin - _before * g_zoom;	// the point under the mouse stays there
		}
		if (_active && (ImGui::IsMouseDragging(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Middle)))
			g_pan = g_pan + _io.MouseDelta;

		if (_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			const std::string _handle = _playing ? std::string() : _handleAt(_mouse);
			const std::string _node = _nodeAt(_mouse);
			if (!_handle.empty()) g_linkFrom = _handle;
			else if (!_node.empty())
			{
				g_selectedNode = _node;
				g_selectedTransition = -1;
				if (!_playing)
				{
					g_dragNode = _node;
					g_dragGrab = _toGraph(_mouse) - PositionOf(_data, _graph.names, _node);
					g_dragMoved = false;
				}
			}
			else
			{
				int _hit = -1;
				for (size_t _i = 0; _i < _arrows.size() && _hit < 0; ++_i)
					if (DistanceToSegment(_mouse, _arrows[_i].a, _arrows[_i].b) < 6.f) _hit = static_cast<int>(_i);
				g_selectedTransition = _hit;
				g_selectedNode.clear();
			}
		}

		if (!g_dragNode.empty() && ImGui::IsMouseDown(ImGuiMouseButton_Left) && (_io.MouseDelta.x != 0.f || _io.MouseDelta.y != 0.f))
		{
			if (!g_dragMoved) Record(_context, "Move animation box", "move." + g_dragNode);
			g_dragMoved = true;
			const ImVec2 _at = _toGraph(_mouse) - g_dragGrab;
			_data.positions[g_dragNode] = FVector2(std::round(_at.x), std::round(_at.y));
		}

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			if (!g_linkFrom.empty())
			{
				const std::string _target = _nodeAt(_mouse);
				if (!_target.empty() && _target != ANY && _target != g_linkFrom)
				{
					Record(_context, "Add transition");
					AnimationTransition _transition;
					_transition.from = g_linkFrom;
					_transition.to = _target;
					_data.transitions.push_back(_transition);
					g_selectedTransition = static_cast<int>(_data.transitions.size()) - 1;
					g_selectedNode.clear();
				}
			}
			g_linkFrom.clear();
			g_dragNode.clear();
		}

		// Right click without moving the view: the menu of the box under the mouse, or of the canvas.
		if (_hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::GetMouseDragDelta(ImGuiMouseButton_Right).x * ImGui::GetMouseDragDelta(ImGuiMouseButton_Right).x
			+ ImGui::GetMouseDragDelta(ImGuiMouseButton_Right).y * ImGui::GetMouseDragDelta(ImGuiMouseButton_Right).y < 16.f)
		{
			g_menuNode = _nodeAt(_mouse);
			ImGui::OpenPopup("##graphMenu");
		}

		if (ImGui::IsWindowFocused() && !_playing && g_selectedTransition >= 0 && ImGui::IsKeyPressed(ImGuiKey_Delete) && !_io.WantTextInput)
		{
			Record(_context, "Delete transition");
			_data.transitions.erase(_data.transitions.begin() + g_selectedTransition);
			g_selectedTransition = -1;
			_arrows.clear();
		}

		// ---- Draw arrows
		for (size_t _i = 0; _i < _arrows.size(); ++_i)
		{
			const Arrow& _arrow = _arrows[_i];
			if (_arrow.a.x < -1e5f) continue;
			const bool _selected = static_cast<int>(_i) == g_selectedTransition;
			const bool _hover = _hovered && g_dragNode.empty() && g_linkFrom.empty() && DistanceToSegment(_mouse, _arrow.a, _arrow.b) < 6.f;
			const ImU32 _color = _selected ? _accent : (_hover ? IM_COL32(230, 230, 235, 255) : IM_COL32(150, 155, 165, 255));
			_draw->AddLine(_arrow.a, _arrow.b, _color, (_selected ? 3.f : 2.f) * std::max(0.6f, g_zoom));

			ImVec2 _direction = _arrow.b - _arrow.a;
			const float _length = std::max(1e-3f, std::sqrt(_direction.x * _direction.x + _direction.y * _direction.y));
			_direction = _direction * (1.f / _length);
			const ImVec2 _side(-_direction.y, _direction.x);
			const ImVec2 _middle = (_arrow.a + _arrow.b) * 0.5f;
			const float _head = 7.f * g_zoom;
			_draw->AddTriangleFilled(_middle + _direction * _head, _middle - _direction * _head + _side * _head, _middle - _direction * _head - _side * _head, _color);

			if (_hover)
			{
				const AnimationTransition& _t = _data.transitions[_i];
				std::string _tip = (_t.from == ANY ? std::string("Any") : _t.from) + " -> " + _t.to;
				if (_t.waitForEnd) _tip += "\nafter its last frame";
				for (const AnimationCondition& _c : _t.conditions)
				{
					const AnimationParameter* _p = _data.FindParameter(_c.parameter);
					_tip += "\n" + _c.parameter + " " + TEST_NAMES[std::clamp(_c.test, 0, 5)];
					if (_p && !IsTruthType(_p->type)) _tip += " " + std::format("{:g}", _c.value);
				}
				if (_t.conditions.empty() && !_t.waitForEnd) _tip += "\n(no condition: at once)";
				ImGui::SetTooltip("%s", _tip.c_str());
			}
		}

		// The transition being drawn
		if (!g_linkFrom.empty())
			_draw->AddLine(_handleOf(g_linkFrom), _mouse, _accent, 2.f * std::max(0.6f, g_zoom));

		// ---- Draw boxes
		const std::string _current = _component.GetCurrentAnimation() ? _component.GetCurrentAnimation()->GetName().StdString() : std::string();
		const std::string _start = _component.startAnimation.StdString().empty() && !_graph.names.empty() ? _graph.names.front() : _component.startAnimation.StdString();
		const float _rounding = 6.f * g_zoom;
		const float _fontSize = ImGui::GetFontSize() * std::clamp(g_zoom, 0.6f, 1.6f);
		for (const std::string& _name : _graph.names)
		{
			const ImVec2 _min = _nodeMin(_name), _max = _min + _nodeSize;
			const bool _isAny = _name == ANY;
			const bool _isCurrent = _playing && _name == _current;
			const bool _selected = _name == g_selectedNode;
			const bool _hover = _hovered && _nodeAt(_mouse) == _name;

			ImU32 _fill = _isAny ? IM_COL32(92, 62, 30, 255) : IM_COL32(48, 51, 58, 255);
			if (_isCurrent) _fill = IM_COL32(40, 120, 70, 255);
			if (_hover) _fill = (EditorTheme::Lighten(_fill) & 0x00FFFFFFu) | (_fill & 0xFF000000u);
			_draw->AddRectFilled(_min, _max, _fill, _rounding);
			if (_name == _start && !_isAny) _draw->AddRectFilled(_min, ImVec2(_min.x + 5.f * g_zoom, _max.y), EditorTheme::GetU32(EditorTheme::Custom::PlayButton), _rounding, ImDrawFlags_RoundCornersLeft);
			_draw->AddRect(_min, _max, _selected ? _accent : IM_COL32(20, 21, 24, 255), _rounding, 0, _selected ? 2.5f : 1.f);

			const std::string _title = _isAny ? "Any animation" : _name;
			const ImVec2 _titleSize = ImGui::GetFont()->CalcTextSizeA(_fontSize, FLT_MAX, 0.f, _title.c_str());
			_draw->AddText(ImGui::GetFont(), _fontSize, ImVec2(_min.x + (_nodeSize.x - _titleSize.x) * 0.5f, _min.y + _nodeSize.y * 0.5f - _titleSize.y * (_isAny ? 0.5f : 0.95f)), IM_COL32(235, 237, 240, 255), _title.c_str());
			if (!_isAny)
			{
				// Frames and loop, small, under the name.
				for (const AnimationClip& _clip : _component.clips)
					if (_clip.name.StdString() == _name)
					{
						const std::string _info = std::to_string(static_cast<int>(_clip.frameCount)) + " frames" + (static_cast<bool>(_clip.loop) ? ", loop" : "") + (_name == _start ? ", start" : "");
						const float _small = _fontSize * 0.8f;
						const ImVec2 _infoSize = ImGui::GetFont()->CalcTextSizeA(_small, FLT_MAX, 0.f, _info.c_str());
						_draw->AddText(ImGui::GetFont(), _small, ImVec2(_min.x + (_nodeSize.x - _infoSize.x) * 0.5f, _min.y + _nodeSize.y * 0.55f), IM_COL32(170, 175, 185, 255), _info.c_str());
						break;
					}
			}

			// The dot to draw a transition from.
			if (!_playing)
			{
				const ImVec2 _handle = _handleOf(_name);
				const bool _overHandle = _hovered && _handleAt(_mouse) == _name;
				_draw->AddCircleFilled(_handle, HANDLE_RADIUS * g_zoom, _overHandle || g_linkFrom == _name ? _accent : IM_COL32(200, 205, 215, 255));
				if (_overHandle && g_linkFrom.empty()) ImGui::SetTooltip("Drag to another animation: a transition");
			}
		}

		if (_graph.names.size() <= 1)
			_draw->AddText(_origin + ImVec2(16.f, 16.f), IM_COL32(200, 200, 205, 255), "No animation yet: make them in the Animation Editor (Tools > Animation Editor).");
		_draw->AddText(ImVec2(_origin.x + 10.f, _origin.y + _size.y - ImGui::GetFontSize() - 8.f), IM_COL32(200, 200, 205, 120),
			"Drag the dot of a box to another: transition   Click an arrow: its conditions   Right mouse: move   Wheel: zoom");
		_draw->PopClipRect();

		// ---- Menu
		if (ImGui::BeginPopup("##graphMenu"))
		{
			if (!g_menuNode.empty() && g_menuNode != ANY)
			{
				ImGui::TextDisabled("%s", g_menuNode.c_str());
				ImGui::BeginDisabled(_playing);
				if (ImGui::MenuItem("Set as starting animation"))
				{
					Record(_context, "Set starting animation");
					_component.startAnimation = String(g_menuNode);
					_component.ApplyClips();
				}
				if (ImGui::BeginMenu("Transition to"))
				{
					for (const std::string& _name : _graph.names)
						if (_name != ANY && _name != g_menuNode && ImGui::MenuItem(_name.c_str()))
						{
							Record(_context, "Add transition");
							_data.transitions.push_back({ g_menuNode, _name, {}, false });
							g_selectedTransition = static_cast<int>(_data.transitions.size()) - 1;
							g_selectedNode.clear();
						}
					ImGui::EndMenu();
				}
				ImGui::EndDisabled();
				if (ImGui::MenuItem("Edit its frames")) OpenAnimationEditor();
				ImGui::Separator();
			}
			if (ImGui::MenuItem("Fit the view"))
			{
				g_zoom = 1.f;
				g_pan = ImVec2(40.f, 40.f);
			}
			ImGui::BeginDisabled(_playing || _data.positions.empty());
			if (ImGui::MenuItem("Arrange the boxes again"))
			{
				Record(_context, "Arrange animation boxes");
				_data.positions.clear();
			}
			ImGui::EndDisabled();
			ImGui::EndPopup();
		}
	}
}

namespace Krampus
{
	void OpenAnimationGraph()
	{
		g_open = true;
		ImGui::SetWindowFocus("Animation Graph");
	}

	void DrawAnimationGraph(EditorContext& _context)
	{
		if (!g_open) return;

		ImGui::SetNextWindowSize(ImVec2(1100.f, 620.f), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin("Animation Graph", &g_open))
		{
			ImGui::End();
			return;
		}

		Graph _graph;
		if (!FindGraph(_context, _graph))
		{
			ImGui::TextWrapped("Select an actor that has an AnimationComponentSM. Its animations (made in the Animation Editor) become boxes "
				"here; drag from the dot of one box to another to make a transition, then give it conditions.");
			ImGui::End();
			return;
		}

		// Another component: its own view and selection.
		if (g_component != _graph.component)
		{
			g_component = _graph.component;
			g_selectedNode.clear();
			g_selectedTransition = -1;
			g_dragNode.clear();
			g_linkFrom.clear();
			g_renaming = -1;
		}
		if (g_selectedTransition >= static_cast<int>(_graph.component->graph.transitions.size())) g_selectedTransition = -1;
		if (!g_selectedNode.empty() && !HasNode(_graph, g_selectedNode)) g_selectedNode.clear();

		const bool _playing = _context.playing;
		ImGui::TextDisabled("%s%s", _graph.owner.c_str(), _playing ? "  (Play: live view, stop to edit the graph)" : "");

		const float _panelWidth = PANEL_WIDTH * ImGui::GetFontSize() / 16.f;
		if (ImGui::BeginChild("##canvasArea", ImVec2(std::max(200.f, ImGui::GetContentRegionAvail().x - _panelWidth - ImGui::GetStyle().ItemSpacing.x), 0.f),
			ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove))
			DrawCanvas(_context, _graph, _playing);
		ImGui::EndChild();

		ImGui::SameLine();
		if (ImGui::BeginChild("##graphPanel", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders))
		{
			if (g_selectedTransition >= 0) DrawTransition(_context, _graph, _playing);
			else if (!g_selectedNode.empty()) DrawNodeDetails(_context, _graph, _playing);
			else ImGui::TextWrapped("Click a box or an arrow to see it here.");
			ImGui::Spacing();
			DrawParameters(_context, _graph, _playing);

			// Transitions about animations that are gone (renamed or deleted in the Animation Editor).
			AnimationGraph& _data = _graph.component->graph;
			const auto _lost = [&](const AnimationTransition& _t) { return !HasNode(_graph, _t.from) || !HasNode(_graph, _t.to); };
			const long long _lostCount = std::count_if(_data.transitions.begin(), _data.transitions.end(), _lost);
			if (_lostCount > 0)
			{
				ImGui::Spacing();
				ImGui::TextColored(EditorTheme::Get(EditorTheme::Custom::ConsoleWarning), "%lld transition(s) name an animation that is gone.", _lostCount);
				ImGui::BeginDisabled(_playing);
				if (ImGui::Button("Delete them", ImVec2(-1.f, 0.f)))
				{
					Record(_context, "Delete lost transitions");
					std::erase_if(_data.transitions, _lost);
					g_selectedTransition = -1;
				}
				ImGui::EndDisabled();
			}
		}
		ImGui::EndChild();

		ImGui::End();
	}
}
