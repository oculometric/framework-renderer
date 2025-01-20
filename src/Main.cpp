#include <windows.h>
#include <string>
#include <map>
#include <iostream>

#include <io.h>
#include <stdio.h>
#include <Windows.h>
#include <fcntl.h>

#define STUI_IMPLEMENTATION
#include "stui.h"

#include "FApplication.h"
#include "ShadowDemoScene.h"
#include "SurrealDemoScene.h"
#include "PhysicsScene.h"
#include "MyScene.h"

#pragma warning( disable : 4996)

template <typename T>
inline FScene* make(FApplication* app, std::string path) { return new T(app, path); }

typedef FScene* (*FSceneFactory)(FApplication*, std::string);

std::map<std::string, FSceneFactory> scene_index = 
{
	{ "MyScene", make<MyScene> }, 
	{ "ShadowDemo", make<ShadowDemoScene> },
	{ "SurrealDemoScene", make<SurrealDemoScene> },
	{ "PhysicsScene", make<PhysicsScene> }
};

FScene* newSceneWithName(std::string name, FApplication* app)
{
	return scene_index[name](app, "res/" + name + ".fscn");
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int fd_std;
	HANDLE h_std;
	CONSOLE_SCREEN_BUFFER_INFO con_info;

	if (_get_osfhandle(0) < 0)
		_close(0);
	freopen("//./NUL", "r", stdin);
	setvbuf(stdin, NULL, _IONBF, 0);
	if (_get_osfhandle(1) < 0)
		_close(1);
	freopen("//./NUL", "w", stdout);
	setvbuf(stdout, NULL, _IONBF, 0);
	if (_get_osfhandle(2) < 0)
		_close(2);
	freopen("//./NUL", "w", stderr);
	setvbuf(stderr, NULL, _IONBF, 0);

	FreeConsole();

	if (!AllocConsole())
	{
		return 1;
	}
	SetConsoleTitle(L"stdout");

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &con_info);
	con_info.dwSize.Y = 1024;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), con_info.dwSize);

	h_std = GetStdHandle(STD_INPUT_HANDLE);
	fd_std = _open_osfhandle((intptr_t)h_std, _O_TEXT);
	_dup2(fd_std, fileno(stdin));
	SetStdHandle(STD_INPUT_HANDLE, (HANDLE)_get_osfhandle(fileno(stdin)));
	_close(fd_std);

	h_std = GetStdHandle(STD_OUTPUT_HANDLE);
	fd_std = _open_osfhandle((intptr_t)h_std, _O_TEXT);
	_dup2(fd_std, fileno(stdout));
	SetStdHandle(STD_OUTPUT_HANDLE, (HANDLE)_get_osfhandle(fileno(stdout)));
	_close(fd_std);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	//SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);

	std::cout << "hello, world" << std::endl;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &con_info);

	stui::Terminal::configure("", 0.5f);

	stui::Renderer::render(new stui::TextArea("fuck yeah", 0)); // TODO: make this into a proper debug window

	FApplication application = FApplication();

	if (FAILED(application.initialise(hInstance, nCmdShow)))
	{
		return -1;
	}

	application.scene = newSceneWithName("PhysicsScene", &application);
	application.scene->start();

	// Main message loop
	MSG msg = { 0 };

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		else
		{
			application.update();
			application.draw();
		}
	}

	return (int)msg.wParam;
}