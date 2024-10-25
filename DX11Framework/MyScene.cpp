#include "MyScene.h"

#include <windows.h>
#include <iostream>

#include "FResourceManager.h"
#include "FApplication.h"

void MyScene::start()
{
	// 99% of the initialisation is now done from the config file! see MyScene.json
	a = findObjectWithName<FMesh>("a");
	b = findObjectWithName<FMesh>("b");
	c = findObjectWithName<FMesh>("c");
}

void MyScene::update(float delta_time)
{
	b->eulers.z -= 180.0f * delta_time;
	b->eulers.y += 36.0f * delta_time;
	c->eulers.x += 24.0f * delta_time;
	c->eulers.y += 36.0f * delta_time;
	c->eulers.z += 12.0f * delta_time;
	root.updateTransform();
	
	if (owner->isFocused() && active_camera != nullptr)
	{
		XMFLOAT4X4 camera_transform = active_camera->getTransform();
		XMFLOAT4 camera_motion = XMFLOAT4
		(
			(float)((GetAsyncKeyState('D') & 0xF000) - (GetAsyncKeyState('A') & 0xF000)),
			(float)((GetAsyncKeyState('E') & 0xF000) - (GetAsyncKeyState('Q') & 0xF000)),
			(float)((GetAsyncKeyState('W') & 0xF000) - (GetAsyncKeyState('S') & 0xF000)),
			0.0f
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
}
