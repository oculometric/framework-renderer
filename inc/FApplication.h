#pragma once

#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <chrono>

#include "FScene.h"

class FGraphicsEngine;

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

	HWND window_handle;

	chrono::steady_clock::time_point time_keeper;
	float total_time = 0.0f;
	float mean_frame_time = 0.0f;

	FGraphicsEngine* engine							= nullptr;

public:
	FScene* scene = nullptr;
	bool needs_viewport_resize = false;

public:
	HRESULT initialise(HINSTANCE hInstance, int nCmdShow);
	HRESULT createWindowHandle(HINSTANCE hInstance, int nCmdShow);
	HRESULT createD3DDevice();

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

	void update();
	void draw();

	~FApplication();
};