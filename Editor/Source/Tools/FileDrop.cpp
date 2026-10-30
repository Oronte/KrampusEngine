#include "FileDrop.h"
#include "Utilities/KrampusInclude.h"
#include <deque>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

using namespace Krampus;

namespace
{
	constexpr UINT_PTR	SUBCLASS_ID	= 0x4B44524F;	// any number of ours: "KDRO"
	HWND				g_window	= nullptr;
	std::deque<FileDrop::Drop>	g_drops;		// filled while SFML reads the window messages, taken by the Content panel

	// Sees the messages of the window before SFML does (a subclass): only WM_DROPFILES is taken, the rest goes on.
	LRESULT CALLBACK DropProc(HWND _window, UINT _message, WPARAM _wParam, LPARAM _lParam, UINT_PTR, DWORD_PTR)
	{
		if (_message != WM_DROPFILES)
			return DefSubclassProc(_window, _message, _wParam, _lParam);

		const HDROP _handle = reinterpret_cast<HDROP>(_wParam);
		FileDrop::Drop _drop;
		const UINT _count = DragQueryFileW(_handle, 0xFFFFFFFF, nullptr, 0);
		for (UINT _i = 0; _i < _count; ++_i)
		{
			const UINT _length = DragQueryFileW(_handle, _i, nullptr, 0);
			std::wstring _path(_length, L'\0');
			DragQueryFileW(_handle, _i, _path.data(), _length + 1);
			_drop.paths.emplace_back(_path);
		}
		POINT _point { };
		DragQueryPoint(_handle, &_point);	// client area pixels
		_drop.position = ImVec2(static_cast<float>(_point.x), static_cast<float>(_point.y));
		DragFinish(_handle);

		if (!_drop.paths.empty()) g_drops.push_back(std::move(_drop));
		return 0;
	}
}

void FileDrop::Install(void* _windowHandle)
{
	Uninstall();
	g_window = static_cast<HWND>(_windowHandle);
	if (!g_window) return;
	if (!SetWindowSubclass(g_window, DropProc, SUBCLASS_ID, 0))
	{
		LOG_WARNING("Editor: files dropped from the Explorer cannot be received (the window refused the subclass)");
		g_window = nullptr;
		return;
	}
	DragAcceptFiles(g_window, TRUE);
}

void FileDrop::Uninstall()
{
	if (!g_window) return;
	DragAcceptFiles(g_window, FALSE);
	RemoveWindowSubclass(g_window, DropProc, SUBCLASS_ID);
	g_window = nullptr;
	g_drops.clear();
}

bool FileDrop::Take(Drop& _drop)
{
	if (g_drops.empty()) return false;
	_drop = std::move(g_drops.front());
	g_drops.pop_front();
	return true;
}
