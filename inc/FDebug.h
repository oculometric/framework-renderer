#pragma once

#include <windows.h>
#include <string>

using namespace std;

class FDebug
{
	friend class FApplication;
private:
	HWND window;

	inline FDebug(HWND window_handle) : window(window_handle) { };
	static void set(FDebug* debug);

public:
	static FDebug* get();

	static void console(string s);
	static void dialog(string s);
};
