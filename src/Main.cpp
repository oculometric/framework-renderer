#include <windows.h>
#include <string>
#include <map>
#include <iostream>

#include "FApplication.h"
#include "ShadowDemoScene.h"
#include "SurrealDemoScene.h"
#include "PhysicsScene.h"
#include "MyScene.h"

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