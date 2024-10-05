#include "MyScene.h"

#include <windows.h>

void MyScene::start()
{
	b.position.x = 1.5f;
	b.scale = XMFLOAT3(0.5f, 0.5f, 0.5f);
	b.eulers.z = 45.0f;
	addObject(&a, nullptr);
	addObject(&b, &a);

	active_camera = new FCamera();
	active_camera->position.z = 5.0f;
	active_camera->eulers.y = 180.0f;
	active_camera->updateProjectionMatrix();
	addObject(active_camera, nullptr);
}

void MyScene::update(float delta_time)
{
	a.eulers.z += 24.0f * delta_time;
	b.eulers.z -= 180.0f * delta_time;
	a.updateTransform();

	// TODO: improve this
    if (GetAsyncKeyState(VK_UP) & 0xF000) active_camera->eulers.x -= delta_time * 3.0f;
    if (GetAsyncKeyState(VK_DOWN) & 0xF000) active_camera->eulers.x += delta_time * 3.0f;
    if (GetAsyncKeyState(VK_LEFT) & 0xF000) active_camera->eulers.z += delta_time * 3.0f;
    if (GetAsyncKeyState(VK_RIGHT) & 0xF000) active_camera->eulers.z -= delta_time * 3.0f;
    if (GetAsyncKeyState('W') & 0xF000) active_camera->position.z += delta_time * 3.0f;
    if (GetAsyncKeyState('S') & 0xF000) active_camera->position.z -= delta_time * 3.0f;
    if (GetAsyncKeyState('A') & 0xF000) active_camera->position.x += delta_time * 3.0f;
    if (GetAsyncKeyState('D') & 0xF000) active_camera->position.x -= delta_time * 3.0f;
	active_camera->updateTransform();
}
