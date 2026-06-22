#define WIN32_LEAN_AND_MEAN

#include "Game.h"

// window related
#include <windows.h>

#include <memory>
#include <stdexcept>

#include <Keyboard.h>

// ! this is IMP
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// -----------------------------------------------------------------------------
// Global variables for now.
// Later we will move these into Game / DeviceResources classes.
// -----------------------------------------------------------------------------

HWND g_window = nullptr;								// window handle, needs a window before we can render it

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

std::unique_ptr<Game> g_game;

// -----------------------------------------------------------------------------
// Window Procedure
// -----------------------------------------------------------------------------

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// DirectXTK Keyboard needs this to receive keyboard messages.
	DirectX::Keyboard::ProcessMessage(message, wParam, lParam);

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

// -----------------------------------------------------------------------------
// Create Win32 window
// -----------------------------------------------------------------------------

void CreateGameWindow(HINSTANCE instance, int showCommand) {
	const wchar_t CLASS_NAME[] = L"DXTKGameWindowClass";

	// create window class instance
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);

	// define window styles
	DWORD windowStyle =
		WS_OVERLAPPED |
		WS_CAPTION |
		WS_SYSMENU |
		WS_MINIMIZEBOX;

	// define window rect
	RECT rect = {};
	rect.left = 0;
	rect.top = 0;
	rect.right = WINDOW_WIDTH;
	rect.bottom = WINDOW_HEIGHT;

	// resize the window
	AdjustWindowRect(&rect, windowStyle, FALSE);

	// calc window size
	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;

	// create the window based on above settings
	g_window = CreateWindowEx(
		0,
		CLASS_NAME,
		L"DirectXTK Game 01",
		windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		instance,
		nullptr
	);

	// throw error if unable to create window
	if (!g_window)
		throw std::runtime_error("Failed to create window.");

	// finally, draw the window
	ShowWindow(g_window, showCommand);
}

// -----------------------------------------------------------------------------
// WinMain
// -----------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
	try {
		CreateGameWindow(instance, showCommand);

		g_game = std::make_unique<Game>();

		g_game->Initialize(g_window, WINDOW_WIDTH, WINDOW_HEIGHT);

		MSG msg = {};

		while (msg.message != WM_QUIT) {
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
				g_game->Tick();
		}
	}
	catch (const std::exception& e) {
		MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
	}

	return 0;
}