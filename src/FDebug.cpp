#include "FDebug.h"

#include <iostream>
#include <Windows.h>

using namespace std;

static FDebug* static_debug_ref = nullptr;

void FDebug::set(FDebug* debug)
{
	static_debug_ref = debug;
}

FDebug* FDebug::get()
{
	return static_debug_ref;
}

void FDebug::console(string s)
{
	//cout << s;
	OutputDebugStringA(s.c_str());
}

void FDebug::dialog(string s)
{
	MessageBoxA(get()->window, s.c_str(), nullptr, ERROR);
}
