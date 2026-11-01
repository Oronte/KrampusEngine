#pragma once
#include <array>
#include <string>
#include <vector>

// Profiler: how long each part of a frame takes (update, physics, render...), for the last frames.
//
//     void Level::Update(...)
//     {
//         PROFILE_SCOPE("Physics");      // from here to the end of the block
//         worldPhysics.Update();
//     }
//
// A scope inside another one is shown under it. A name used several times in a frame adds up. The editor shows it in
// Tools > Profiler.

namespace Krampus
{
	class Profiler
	{
	public:
		static constexpr int HISTORY = 240;		// frames kept

		struct Section
		{
			std::string						name;
			int								depth		= 0;	// 0: a part of the frame, 1: a part of a part...
			std::array<float, HISTORY>		history		{ };	// milliseconds, per frame
			int								calls		= 0;	// scopes in the last frame
			float							current		= 0.f;	// milliseconds so far in this frame
			int								currentCalls = 0;
		};

		/// <summary>Starts a timed part. Prefer PROFILE_SCOPE, which ends it at the end of the block.</summary>
		static void Begin(const char* _name);

		/// <summary>Ends the part started last.</summary>
		static void End();

		/// <summary>The frame is over: each part's time goes in its history (the engine calls it at the end of each Tick).</summary>
		static void EndFrame();

		static const std::vector<Section>& GetSections();

		/// <summary>The whole frame, milliseconds, per frame.</summary>
		static const std::array<float, HISTORY>& GetFrameTimes();

		/// <summary>The slot of the history the last frame was written in.</summary>
		static int GetLastFrame();

		/// <summary>Average and highest of the last _frames frames.</summary>
		static float Average(const std::array<float, HISTORY>& _history, int _frames = 60);
		static float Highest(const std::array<float, HISTORY>& _history, int _frames = 60);

		/// <summary>Forgets every part (they come back when they run again).</summary>
		static void Clear();

		/// <summary>When true the history stops moving, to look at it.</summary>
		static inline bool paused = false;
	};

	struct ProfileScope
	{
		explicit ProfileScope(const char* _name) { Profiler::Begin(_name); }
		~ProfileScope() { Profiler::End(); }
		ProfileScope(const ProfileScope&) = delete;
		ProfileScope& operator=(const ProfileScope&) = delete;
	};
}

#define KRAMPUS_PROFILE_JOIN2(_a, _b) _a##_b
#define KRAMPUS_PROFILE_JOIN(_a, _b) KRAMPUS_PROFILE_JOIN2(_a, _b)
#define PROFILE_SCOPE(_name) ::Krampus::ProfileScope KRAMPUS_PROFILE_JOIN(_profileScope, __LINE__)(_name)
