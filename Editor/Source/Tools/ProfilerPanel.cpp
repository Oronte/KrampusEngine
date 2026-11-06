#include "EditorContext.h"
#include "EditorWidgets.h"
#include "Utilities/Debug/Profiler.h"
#include <imgui.h>
#include <algorithm>

using namespace Krampus;

// Tools > Profiler: how long each part of a frame takes (PROFILE_SCOPE in the code), averaged on the last frames, with the
// highest time and a graph of the whole frame. The parts of the level (Actors, Physics...) only run in Play.

namespace
{
	bool	g_open		= false;
	int		g_frames	= 60;		// frames the average and the highest are taken on

	// A bar as long as the part's share of the frame.
	void ShareBar(const float _share)
	{
		const ImVec2 _start = ImGui::GetCursorScreenPos();
		const float _width = ImGui::GetContentRegionAvail().x;
		const float _height = ImGui::GetTextLineHeight();
		ImDrawList* _draw = ImGui::GetWindowDrawList();
		_draw->AddRectFilled(_start, ImVec2(_start.x + _width, _start.y + _height), ImGui::GetColorU32(ImGuiCol_FrameBg), 2.f);
		_draw->AddRectFilled(_start, ImVec2(_start.x + _width * std::clamp(_share, 0.f, 1.f), _start.y + _height), ImGui::GetColorU32(ImGuiCol_PlotHistogram), 2.f);
		ImGui::Dummy(ImVec2(_width, _height));
	}
}

namespace Krampus
{
	void OpenProfiler()
	{
		g_open = true;
		ImGui::SetWindowFocus("Profiler");
	}

	void DrawProfiler(EditorContext& _context)
	{
		if (!g_open) return;
		if (!ImGui::Begin("Profiler", &g_open))
		{
			ImGui::End();
			return;
		}

		const std::array<float, Profiler::HISTORY>& _frameTimes = Profiler::GetFrameTimes();
		const float _frame = Profiler::Average(_frameTimes, g_frames);
		ImGui::Text("Frame: %.2f ms (%.0f FPS)   highest %.2f ms", _frame, _frame > 0.f ? 1000.f / _frame : 0.f, Profiler::Highest(_frameTimes, g_frames));
		if (!_context.playing) ImGui::TextDisabled("Edit mode: the level does not run (press Play for Actors, Physics...)");

		ImGui::Checkbox("Pause", &Profiler::paused);
		ImGui::SameLine();
		if (ImGui::Button("Clear")) Profiler::Clear();
		ImGui::SameLine();
		ImGui::SetNextItemWidth(140.f);
		EditorUI::SliderInt("Frames", &g_frames, 1, Profiler::HISTORY);

		// The whole frame, oldest on the left.
		std::array<float, Profiler::HISTORY> _ordered;
		for (int _i = 0; _i < Profiler::HISTORY; ++_i) _ordered[_i] = _frameTimes[(Profiler::GetLastFrame() + 1 + _i) % Profiler::HISTORY];
		const float _top = std::max(16.7f, Profiler::Highest(_frameTimes, Profiler::HISTORY) * 1.1f);
		ImGui::PlotLines("##frames", _ordered.data(), Profiler::HISTORY, 0, "frame (ms)", 0.f, _top, ImVec2(-FLT_MIN, 70.f));

		if (ImGui::BeginTable("##sections", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("Part", ImGuiTableColumnFlags_WidthStretch, 2.f);
			ImGui::TableSetupColumn("ms", ImGuiTableColumnFlags_WidthFixed, 60.f);
			ImGui::TableSetupColumn("highest", ImGuiTableColumnFlags_WidthFixed, 60.f);
			ImGui::TableSetupColumn("calls", ImGuiTableColumnFlags_WidthFixed, 45.f);
			ImGui::TableSetupColumn("share of the frame", ImGuiTableColumnFlags_WidthStretch, 1.5f);
			ImGui::TableHeadersRow();

			for (const Profiler::Section& _section : Profiler::GetSections())
			{
				const float _average = Profiler::Average(_section.history, g_frames);
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Indent(_section.depth * 14.f);
				ImGui::TextUnformatted(_section.name.c_str());
				ImGui::Unindent(_section.depth * 14.f);
				ImGui::TableNextColumn();
				ImGui::Text("%.3f", _average);
				ImGui::TableNextColumn();
				ImGui::Text("%.3f", Profiler::Highest(_section.history, g_frames));
				ImGui::TableNextColumn();
				ImGui::Text("%d", _section.calls);
				ImGui::TableNextColumn();
				ShareBar(_frame > 0.f ? _average / _frame : 0.f);
			}
			ImGui::EndTable();
		}
		ImGui::End();
	}
}
