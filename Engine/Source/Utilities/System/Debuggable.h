#pragma once

namespace Krampus
{

	class Window;

	/// <summary>
	/// Interface for debuggable objects.
	/// </summary>
	class IDebuggable
	{
	public:
		bool useDebug = false;

	protected:
		virtual void DrawDebug(Window& _window) = 0;
	};

}

/////////////////////////////////////////////////
// 
// class DebugableClass : public IDebuggable 
// {
//     virtual void DrawDebug(Window& _window) override
//     {
//		#if DEBUG
// 			if (!useDebug) return;
//         _window.Draw(objectToDebug);
//		#endif
//     }
// }
// 
/////////////////////////////////////////////////