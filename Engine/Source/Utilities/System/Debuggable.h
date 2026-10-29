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
		/// <summary>
		/// Called by the CameraManager every frame for every debuggable component: draws the outline when debugging is on.
		/// </summary>
		void RenderDebug()
		{
			if (useDebug) DrawDebug();
		}

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