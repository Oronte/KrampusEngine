#pragma once

namespace Krampus
{

	class Window;

	/// <summary>
	/// Interface for debuggable objects.
	/// </summary>
	class IDebuggable
	{
		Bool useDebug = false;

	protected:
		virtual void DrawDebug() = 0;

	public:
		Bool IsDebugging() const
		{
			return useDebug;
		}
		void UseDebug()
		{
			useDebug = true;
		}
		void DisableDebug()
		{
			useDebug = false;
		}
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