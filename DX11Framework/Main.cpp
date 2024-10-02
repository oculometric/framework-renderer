#include <windows.h>
#include "FApplication.h"
#include "FScene.h"

//Dependencies:user32.lib;d3d11.lib;d3dcompiler.lib;dxgi.lib;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	FApplication application = FApplication();
	application.scene = new FScene(&application);
	FObject a = FObject();
	FObject b = FObject();
	b.position.x = 1.5f;
	b.eulers.z = 45.0f;
	application.scene->addObject(&a, nullptr);
	application.scene->addObject(&b, &a);

	if (FAILED(application.initialise(hInstance, nCmdShow)))
	{
		return -1;
	}

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
			a.eulers.z += 0.1f;
			a.updateTransform();
			application.update();
			application.draw();
		}
	}

	return (int)msg.wParam;
}