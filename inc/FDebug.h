#pragma once

#include <windows.h>
#include <string>

// encapsulates easy debug output and alerts
class FDebug
{
	friend class FApplication;
private:
	HWND window;

	inline FDebug(HWND window_handle) : window(window_handle) { };
	static void set(FDebug* debug);			// assigns an instance pointer to the static reference in the source file

public:
	static FDebug* get();					// gets the value of the static reference from the source file

	static void console(std::string s);		// prints a string to both stdout and the visual studio immediate window
	static void dialog(std::string s); 		// shows a string as an onscreen dialog box
};
