#pragma once

namespace Krampus
{

	class Engine;

	/// <summary>
	/// Entry point of the game code: called once the engine is created, it chooses the first level.
	/// The editor (or any other host) only knows the game through this function.
	/// </summary>
	void GameStart(Engine& _engine);
}
