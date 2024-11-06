#pragma once

#include <windows.h>
#include <string>

class FDebug
{
	friend class FApplication;
private:
	HWND window;

	inline FDebug(HWND window_handle) : window(window_handle) { };
	static void set(FDebug* debug);

public:
	static FDebug* get();

	static void console(std::string s);
	static void dialog(std::string s);
};
