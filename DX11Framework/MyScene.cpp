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
	b->transform.rotate(XMFLOAT3(0, 0, 1), 180.0f * delta_time, b->transform.getPosition());
	c->transform.rotate(XMFLOAT3(1, 0, 0), 24.0f * delta_time, c->transform.getPosition());
	c->transform.rotate(XMFLOAT3(0, 1, 0), 36.0f * delta_time, c->transform.getPosition());
	c->transform.rotate(XMFLOAT3(0, 0, 1), 12.0f * delta_time, c->transform.getPosition());
	
	if (owner->isFocused() && active_camera != nullptr)
	{
		XMFLOAT4X4 camera_transform = active_camera->transform.getTransform();
		XMFLOAT4 camera_motion = XMFLOAT4
		(
			(float)((GetAsyncKeyState('D') & 0xF000) - (GetAsyncKeyState('A') & 0xF000)),
			(float)((GetAsyncKeyState('E') & 0xF000) - (GetAsyncKeyState('Q') & 0xF000)),
			(float)((GetAsyncKeyState('S') & 0xF000) - (GetAsyncKeyState('W') & 0xF000)),
			0.0f
		);

		FLOAT speed = GetAsyncKeyState(VK_SHIFT) & 0xF000 ? 8.0f : 3.0f;

		FLOAT up_down = (float)(((GetAsyncKeyState(VK_UP) & 0xF000) > 0) - ((GetAsyncKeyState(VK_DOWN) & 0xF000) > 0));
		FLOAT left_right = (float)(((GetAsyncKeyState(VK_LEFT) & 0xF000) > 0) - ((GetAsyncKeyState(VK_RIGHT) & 0xF000) > 0));

		active_camera->transform.rotate(XMFLOAT3(0, 0, 1), left_right * 60.0f * delta_time, active_camera->transform.getPosition());
		active_camera->transform.rotate(active_camera->transform.getRight(), up_down * 60.0f * delta_time, active_camera->transform.getPosition());

		XMFLOAT3 delta;
		XMStoreFloat3
		(
			&delta,
			XMVector4Transform
			(
				XMVector4Normalize(XMLoadFloat4(&camera_motion)) * delta_time * speed,
				XMLoadFloat4x4(&camera_transform)
			)
		);
		active_camera->transform.translate(delta);

		if (GetAsyncKeyState(VK_LBUTTON) & 0xF000)
		{
			POINT p;
			GetCursorPos(&p);
			RECT r;
			GetWindowRect(owner->getWindow(), &r);
			XMFLOAT2 screen_pos = XMFLOAT2((float)(p.x - r.left) / (float)(r.right - r.left), (float)(p.y - r.top) / (float)(r.bottom - r.top));
			XMFLOAT4 clip_pos = XMFLOAT4(screen_pos.x * 2.0f - 1.0f, (screen_pos.y * 2.0f - 1.0f) * -1.0f, 0, 1);
			XMFLOAT4X4 proj = active_camera->getProjectionMatrix();
			XMFLOAT4X4 view = active_camera->transform.getTransform();
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

			float inv_direction[3] = { 1.0f / world_direction.x, 1.0f / world_direction.y, 1.0f / world_direction.z };
			float origin[3] = { view._41, view._42, view._43 };

			FObject* closest = nullptr;
			float dist = INFINITY;

			for (FObject* obj : all_objects)
			{
				if (obj->getType() != FObjectType::MESH) continue;
				FMesh* m = (FMesh*)obj;
				FBoundingBox box = m->getWorldSpaceBounds();

				// based closely on https://psgraphics.blogspot.com/2016/02/new-simple-ray-box-test-from-andrew.html
				float mins[3] = { box.min_corner.x, box.min_corner.y, box.min_corner.z };
				float maxs[3] = { box.max_corner.x, box.max_corner.y, box.max_corner.z };

				float tmin = 0;
				float tmax = 1000;
				bool failed = false;

				for (int i = 0; i < 3; i++)
				{
					float t0 = (mins[i] - origin[i]) * inv_direction[i];
					float t1 = (maxs[i] - origin[i]) * inv_direction[i];
					if (inv_direction[i] < 0)
						swap(t0, t1);
					tmin = max(t0, tmin);
					tmax = min(t1, tmax);

					if (tmax < tmin) { failed = true; break; }
				}

				if (!failed && tmin < dist)
				{
					closest = obj;
					dist = tmin;
				}
			}

			active_object = closest;
		}
	}
}
