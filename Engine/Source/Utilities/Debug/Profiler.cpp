#include "Profiler.h"
#include <algorithm>
#include <chrono>

using namespace Krampus;

namespace
{
	using Clock = std::chrono::steady_clock;

	struct Open
	{
		size_t				section;
		Clock::time_point	start;
	};

	std::vector<Profiler::Section>				g_sections;
	std::vector<Open>							g_open;		// the scopes running now, inner last
	std::array<float, Profiler::HISTORY>		g_frames	{ };
	int											g_last		= 0;
	Clock::time_point							g_frameStart = Clock::now();
	bool										g_clear		= false;	// Clear() asked: done between two frames

	float Milliseconds(const Clock::duration& _duration)
	{
		return std::chrono::duration<float, std::milli>(_duration).count();
	}
}

void Profiler::Begin(const char* _name)
{
	const int _depth = static_cast<int>(g_open.size());
	size_t _index = 0;
	for (; _index < g_sections.size(); ++_index)
		if (g_sections[_index].depth == _depth && g_sections[_index].name == _name) break;
	if (_index == g_sections.size())
	{
		// A new part goes right after the last part of its parent, so the list reads like the frame.
		Section _section;
		_section.name = _name;
		_section.depth = _depth;
		size_t _at = g_sections.size();
		if (!g_open.empty())
		{
			_at = g_open.back().section + 1;
			const int _parentDepth = g_sections[g_open.back().section].depth;
			while (_at < g_sections.size() && g_sections[_at].depth > _parentDepth) ++_at;
		}
		g_sections.insert(g_sections.begin() + _at, std::move(_section));
		for (Open& _open : g_open) if (_open.section >= _at) ++_open.section;
		_index = _at;
	}
	g_open.push_back({ _index, Clock::now() });
}

void Profiler::End()
{
	if (g_open.empty()) return;
	const Open _open = g_open.back();
	g_open.pop_back();
	Section& _section = g_sections[_open.section];
	_section.current += Milliseconds(Clock::now() - _open.start);
	++_section.currentCalls;
}

void Profiler::EndFrame()
{
	if (g_clear && g_open.empty())
	{
		g_clear = false;
		g_sections.clear();
		g_frames.fill(0.f);
	}
	const Clock::time_point _now = Clock::now();
	const float _frame = Milliseconds(_now - g_frameStart);
	g_frameStart = _now;
	if (!paused)
	{
		g_last = (g_last + 1) % HISTORY;
		g_frames[g_last] = _frame;
	}
	for (Section& _section : g_sections)
	{
		if (!paused)
		{
			_section.history[g_last] = _section.current;
			_section.calls = _section.currentCalls;
		}
		_section.current = 0.f;
		_section.currentCalls = 0;
	}
}

const std::vector<Profiler::Section>& Profiler::GetSections()
{
	return g_sections;
}

const std::array<float, Profiler::HISTORY>& Profiler::GetFrameTimes()
{
	return g_frames;
}

int Profiler::GetLastFrame()
{
	return g_last;
}

float Profiler::Average(const std::array<float, HISTORY>& _history, int _frames)
{
	_frames = std::clamp(_frames, 1, HISTORY);
	float _sum = 0.f;
	for (int _i = 0; _i < _frames; ++_i) _sum += _history[(g_last - _i + HISTORY) % HISTORY];
	return _sum / static_cast<float>(_frames);
}

float Profiler::Highest(const std::array<float, HISTORY>& _history, int _frames)
{
	_frames = std::clamp(_frames, 1, HISTORY);
	float _highest = 0.f;
	for (int _i = 0; _i < _frames; ++_i) _highest = std::max(_highest, _history[(g_last - _i + HISTORY) % HISTORY]);
	return _highest;
}

void Profiler::Clear()
{
	g_clear = true;	// the scopes running now still point into the list
}
