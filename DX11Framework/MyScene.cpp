#include "MyScene.h"

#include <windows.h>
#include <iostream>

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
	
	XMFLOAT4X4 camera_transform = active_camera->getTransform();
	XMFLOAT4 camera_motion = 
	XMFLOAT4(
		(GetAsyncKeyState('D') & 0xF000) - (GetAsyncKeyState('A') & 0xF000),
		(GetAsyncKeyState('E') & 0xF000) - (GetAsyncKeyState('Q') & 0xF000),
		(GetAsyncKeyState('W') & 0xF000) - (GetAsyncKeyState('S') & 0xF000),
		0
	);

	FLOAT speed = GetAsyncKeyState(VK_SHIFT) & 0xF000 ? 8.0f : 3.0f;

    if (GetAsyncKeyState(VK_UP) & 0xF000) active_camera->eulers.x += delta_time * 60.0f;
    if (GetAsyncKeyState(VK_DOWN) & 0xF000) active_camera->eulers.x -= delta_time * 60.0f;
    if (GetAsyncKeyState(VK_LEFT) & 0xF000) active_camera->eulers.z -= delta_time * 60.0f;
    if (GetAsyncKeyState(VK_RIGHT) & 0xF000) active_camera->eulers.z += delta_time * 60.0f;
	
	XMStoreFloat3
	(
		&active_camera->position, 
		XMVector4Transform
		(
			XMVector4Normalize(XMLoadFloat4(&camera_motion)) * delta_time * speed,
			XMLoadFloat4x4(&camera_transform)
		) + XMLoadFloat3(&active_camera->position)
	);
	active_camera->updateTransform();
}
