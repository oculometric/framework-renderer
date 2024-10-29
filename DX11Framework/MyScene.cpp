#include "MyScene.h"

#include <windows.h>
#include <iostream>

#include "FResourceManager.h"
#include "FApplication.h"
#include "FDebug.h"

void MyScene::start()
{
	// 99% of the initialisation is now done from the config file! see MyScene.json
	a = findObjectWithName<FMesh>("a");
	b = findObjectWithName<FMesh>("sphere");
	c = findObjectWithName<FMesh>("c");
}

void MyScene::update(float delta_time)
{
	b->eulers.z -= 180.0f * delta_time;
	//b->eulers.y += 36.0f * delta_time;
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
			(float)((GetAsyncKeyState('S') & 0xF000) - (GetAsyncKeyState('W') & 0xF000)),
			0.0f
		);

		FLOAT speed = GetAsyncKeyState(VK_SHIFT) & 0xF000 ? 8.0f : 3.0f;

		if (GetAsyncKeyState(VK_UP) & 0xF000) active_camera->eulers.x -= delta_time * 60.0f;
		if (GetAsyncKeyState(VK_DOWN) & 0xF000) active_camera->eulers.x += delta_time * 60.0f;
		if (GetAsyncKeyState(VK_LEFT) & 0xF000) active_camera->eulers.z += delta_time * 60.0f;
		if (GetAsyncKeyState(VK_RIGHT) & 0xF000) active_camera->eulers.z -= delta_time * 60.0f;

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

	FObject* demo = findObjectWithName<FObject>("backing");
	if (GetAsyncKeyState('U')) demo->eulers.x += delta_time * 100.0f;
	if (GetAsyncKeyState('I')) demo->eulers.x -= delta_time * 100.0f;
	if (GetAsyncKeyState('H')) demo->eulers.y += delta_time * 100.0f;
	if (GetAsyncKeyState('J')) demo->eulers.y -= delta_time * 100.0f;
	if (GetAsyncKeyState('B')) demo->eulers.z += delta_time * 100.0f;
	if (GetAsyncKeyState('N')) demo->eulers.z -= delta_time * 100.0f;
	demo->updateTransform();

	if (GetAsyncKeyState(VK_LBUTTON) & 0xF000)
	{
		POINT p;
		GetCursorPos(&p);
		RECT r;
		GetWindowRect(owner->getWindow(), &r);
		XMFLOAT2 screen_pos = XMFLOAT2((float)(p.x - r.left) / (float)(r.right - r.left), (float)(p.y - r.top) / (float)(r.bottom - r.top));
		XMFLOAT4 clip_pos = XMFLOAT4(screen_pos.x * 2.0f - 1.0f, (screen_pos.y * 2.0f - 1.0f) * -1.0f, 0, 1);
		XMFLOAT4X4 proj = active_camera->getProjectionMatrix();
		XMFLOAT4X4 view = active_camera->getTransform();
		XMFLOAT3 world_direction;
		XMStoreFloat3
		(
			&world_direction,
			XMVector3Normalize
			(
				XMVector4Transform
				(
					XMVector4Transform
					(
						XMLoadFloat4(&clip_pos),
						XMMatrixInverse(nullptr, XMLoadFloat4x4(&proj))
					) * XMVectorSet(1, 1, 1, 0),
					XMLoadFloat4x4(&view)
				)
			)
		);
		FDebug::dialog(string(to_string(world_direction.x) + ", " + to_string(world_direction.y) + ", " + to_string(world_direction.z)));

		float inv_direction[3] = { 1.0f / world_direction.x, 1.0f / world_direction.y, 1.0f / world_direction.z };
		float origin[3] = { view._41, view._42, view._43 }; // TODO: here

		for (FObject* obj : all_objects)
		{
			if (obj->getType() != FObjectType::MESH) continue;
			FMesh* m = (FMesh*)obj;
			FBoundingBox b = m->getWorldSpaceBounds();

			float mins[3] = { b.min_corner.x, b.min_corner.y, b.min_corner.z };
			float maxs[3] = { b.max_corner.x, b.max_corner.y, b.max_corner.z };

			for (int i = 0; i < 3; i++)
			{
				float t0 = mins[i] - 
			}
		}
	}
}
