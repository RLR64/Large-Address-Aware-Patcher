#ifndef GUI_HPP
#define GUI_HPP

// STL
#include <string>

// Windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <commctrl.h>

// Local
#include "patcher.hpp"

namespace gui {

	// Represents a single dropped EXE in the list
	struct FileEntry {
		std::wstring path;
		std::wstring filename;
		std::wstring directory;
		bool         backup{ true };
		laa::Info    info;
		laa::Result  checkResult;
		std::wstring statusText;
	};

	// Initialises and shows the main window. Returns the WinMain exit code.
	auto run(HINSTANCE hInstance, int nCmdShow) -> int;

} // namespace gui

#endif // GUI_HPP