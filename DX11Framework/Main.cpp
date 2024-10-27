#include <windows.h>

#include "FApplication.h"
#include "PlanetScene.h"
#include "MyScene.h"
#include "FResourceManager.h"

//Dependencies:user32.lib;d3d11.lib;d3dcompiler.lib;dxgi.lib;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	FApplication application = FApplication();

	if (FAILED(application.initialise(hInstance, nCmdShow)))
	{
		return -1;
	}

	//application.scene = new PlanetScene(&application, "PlanetScene.fscn");
	application.scene = new MyScene(&application, "MyScene.fscn");
	application.scene->finalizePreload();
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