// Windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

// Local
#include "gui.hpp"

int WINAPI wWinMain(
	_In_     HINSTANCE hInstance,
	_In_opt_ HINSTANCE /*hPrevInstance*/,
	_In_     LPWSTR    /*lpCmdLine*/,
	_In_     int       nCmdShow
) {
	return gui::run(hInstance, nCmdShow);
}