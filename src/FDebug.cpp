#define STUI_IMPLEMENTATION
#include "FDebug.h"

#include <iostream>
#include <Windows.h>
#include <io.h>
#include <stdio.h>
#include <fcntl.h>

using namespace std;
using namespace stui;

static FDebug* static_debug_ref = nullptr;

#pragma warning( disable : 4996)

void quit_callback()
{
	FDebug::console("quitting!");
	PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 0, 0);
}

FDebug::FDebug(HWND window_handle)
{
	window = window_handle;

	// initialise a new terminal window for STUI to use
	int fd_std;
	HANDLE h_std;
	CONSOLE_SCREEN_BUFFER_INFO con_info;

	// destroy the old terminal
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

	// create and configure the new one
	if (!AllocConsole())
	{
		return;
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

	// STUI initialisation
	Terminal::configure("framework renderer", 0.5f);
	page = new Page();

	log_text_area = new TextArea("Hello, World!", 0);

	frame_time_label = new Label("unset", 1);
	framerate_label = new Label("unset", 1);
	clear_timing_label = new Label("unset", 1);
	shadows_timing_label = new Label("unset", 1);
	uniforms_timing_label = new Label("unset", 1);
	batching_timing_label = new Label("unset", 1);
	objects_timing_label = new Label("unset", 1);
	postprocessing_timing_label = new Label("unset", 1);
	gizmos_timing_label = new Label("unset", 1);
	objects_count_label = new Label("unset", 1);
	meshes_count_label = new Label("unset", 1);
	triangles_drawn_label = new Label("unset", 1);
	lights_count_label = new Label("unset", 1);
	camera_forward_label = new Label("unset", 1);
	render_stats_box = new VerticalBox(
		{
			new HorizontalBox({ new Label("frame time", -1), frame_time_label }),
			new HorizontalBox({ new Label("framerate", -1), framerate_label }),
			new Label("sub timings:", -1),
			new HorizontalBox({ new Label("  clear", -1), clear_timing_label }),
			new HorizontalBox({ new Label("  shadows", -1), shadows_timing_label }),
			new HorizontalBox({ new Label("  uniforms", -1), uniforms_timing_label }),
			new HorizontalBox({ new Label("  batching", -1), batching_timing_label }),
			new HorizontalBox({ new Label("  objects", -1), objects_timing_label }),
			new HorizontalBox({ new Label("  postprocessing", -1), postprocessing_timing_label }),
			new HorizontalBox({ new Label("  gizmos", -1), gizmos_timing_label }),
			new HorizontalBox({ new Label("objects in scene", -1), objects_count_label }),
			new HorizontalBox({ new Label("  meshes drawn", -1), meshes_count_label }),
			new HorizontalBox({ new Label("  triangles drawn", -1), triangles_drawn_label }),
			new HorizontalBox({ new Label("  lights", -1), lights_count_label }),
			new HorizontalBox({ new Label("camera forward", -1), camera_forward_label })
		}
	);

	tick_time_label = new Label("unset", 1);
	components_count_label = new Label("unset", 1);
	physics_stats_box = new VerticalBox(
		{
			new HorizontalBox({ new Label("tick time", -1), tick_time_label}),
			new HorizontalBox({ new Label("components", -1), components_count_label})
		}
	);

	extra_info = new ListView({}, 0, 0);

	HorizontalBox* hb = new HorizontalBox
	({
		new SizeLimiter
		(
			new VerticalBox
			({ 
				new BorderedBox(render_stats_box, "render stats"),
				new BorderedBox(physics_stats_box, "physics stats"),
				extra_info
			})
			, Coordinate{ 32, -1 }
		),
		new BorderedBox(log_text_area, "console")
	});

	spinner = new Spinner(0, 0);
	quit_button = new Button("quit", quit_callback, true);
	VerticalBox* vb = new VerticalBox
	({
		new HorizontalBox({ new Label("FRAMEWORK STATS", -1), spinner, quit_button }),
		hb
	});

	page->setRoot(vb);

	page->focusable_component_sequence.push_back(log_text_area);
	page->focusable_component_sequence.push_back(extra_info);
	page->focusable_component_sequence.push_back(quit_button);
}

void FDebug::set(FDebug* debug)
{
	static_debug_ref = debug;
}

FDebug* FDebug::get()
{
	return static_debug_ref;
}

void FDebug::update()
{
	get()->spinner->state++;
	get()->page->checkInput();
	get()->page->render();
}

void FDebug::console(string s)
{
	get()->log_text_area->text += "\n" + s;
	get()->log_text_area->scroll++;
	OutputDebugStringA(s.c_str());
}

void FDebug::dialog(string s)
{
	MessageBoxA(get()->window, s.c_str(), nullptr, ERROR);
}
