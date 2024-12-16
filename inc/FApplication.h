#pragma once

#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <chrono>

#include "FScene.h"

class FPhysicsEngine;
class FGraphicsEngine;

// manages the windows, devices, timekeeping, and graphics engine
class FApplication
{
private:
	int window_width = 640;
	int window_height = 480;
	int window_x = 0;
	int window_y = 0;

	ID3D11DeviceContext* immediate_context			= nullptr;
	ID3D11Device* device							= nullptr;
	IDXGIDevice* dxgi_device						= nullptr;
	IDXGIFactory2* dxgi_factory						= nullptr;

	HWND window_handle;			// handle for the main game window

	HWND info_window_handle;	// handle for the stats window

	// used to keep track of the frame delta-time
	std::chrono::steady_clock::time_point time_keeper;
	float total_time = 0.0f;
	float mean_frame_time = 0.0f;

	// reference to the graphics engine
	FGraphicsEngine* engine							= nullptr;
	// reference to the physics engine
	FPhysicsEngine* physics_engine					= nullptr;

public:
	FScene* scene = nullptr;				// active scene
	bool needs_viewport_resize = false;		// whether or not the window has been resized and thus graphics resources like the framebuffer need to also be resized

public:
	HRESULT initialise(HINSTANCE hInstance, int nCmdShow);			// configures the application, device, context, and graphics engine
	HRESULT createWindowHandle(HINSTANCE hInstance, int nCmdShow);
	HRESULT createD3DDevice();

	// getters for various properties/states
	inline ID3D11DeviceContext* getContext() { return immediate_context; }
	inline ID3D11Device* getDevice() { return device; }
	inline IDXGIFactory2* getFactory() { return dxgi_factory; }
	inline float getTime() { return total_time; }
	void updateWindowSize();
	inline float getWidth() { return (float)window_width; }
	inline float getHeight() { return (float)window_height; }
	inline float getX() { return (float)window_x; }
	inline float getY() { return (float)window_y; }
	inline HWND getWindow() { return window_handle; }
	inline FGraphicsEngine* getEngine() { return engine; }
	inline bool isFocused() { return GetFocus() == window_handle; }
	inline void clearFocus() { SetFocus(info_window_handle); }

	// set the text content of the stats window
	void updateStats(std::wstring str);

	// update and draw methods which will be passed on to the graphics engine and the scene
	void update();
	void draw();

	~FApplication();
};