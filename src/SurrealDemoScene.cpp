#include "SurrealDemoScene.h"

#include <windows.h>
#include <iostream>

#include "FResourceManager.h"
#include "FApplication.h"
#include "FDebug.h"
#include "FGraphicsEngine.h"

void SurrealDemoScene::start()
{
	orrery_base = findObjectWithName<FObject>("orrery_base");
	orrery_mid = findObjectWithName<FObject>("orrery_mid");
	orrery_planet_a = findObjectWithName<FObject>("orrery_planet_a");
	orrery_planet_b = findObjectWithName<FObject>("orrery_planet_b");
	orrery_core = findObjectWithName<FObject>("orrery_core");

	fly_cam = findObjectWithName<FCamera>("fly_cam");
	walk_cam = findObjectWithName<FCamera>("walk_cam");
	active_camera = fly_mode ? fly_cam : walk_cam;

	getMouseDeltaAndReset();
}

void SurrealDemoScene::update(float delta_time)
{
	XMFLOAT2 mouse_delta = getMouseDeltaAndReset();

	orrery_mid->transform.rotate(orrery_mid->transform.getRight(), delta_time * 30.0f, orrery_mid->transform.getPosition());
	orrery_core->transform.rotate(orrery_core->transform.getForward(), delta_time * 45.0f, orrery_core->transform.getPosition());
	orrery_planet_a->transform.rotate(orrery_planet_a->transform.getForward(), delta_time * 10.0f, orrery_planet_a->transform.getPosition());
	orrery_planet_b->transform.rotate(orrery_planet_b->transform.getForward(), delta_time * 2.0f, orrery_planet_b->transform.getPosition());

	if (owner->isFocused() && active_camera != nullptr)
	{
		if (GetAsyncKeyState('1') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::POST_PROCESS;
		if (GetAsyncKeyState('2') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SCENE_COLOUR;
		if (GetAsyncKeyState('3') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SCENE_NORMAL;
		if (GetAsyncKeyState('4') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SCENE_DEPTH;
		if (GetAsyncKeyState('5') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SHARPENED;

		if (GetAsyncKeyState(VK_TAB) & 0x0001)
		{
			fly_mode = !fly_mode;
			owner->getEngine()->draw_gizmos = fly_mode;
			active_camera = fly_mode ? fly_cam : walk_cam;
		}

		XMFLOAT4X4 camera_transform = active_camera->transform.getTransform();
		XMFLOAT4 camera_motion = XMFLOAT4
		(
			(float)((GetAsyncKeyState('D') & 0xF000) - (GetAsyncKeyState('A') & 0xF000)),
			(float)((GetAsyncKeyState('E') & 0xF000) - (GetAsyncKeyState('Q') & 0xF000)),
			(float)((GetAsyncKeyState('S') & 0xF000) - (GetAsyncKeyState('W') & 0xF000)),
			0.0f
		);

		if (abs(camera_motion.x) > 0 || abs(camera_motion.y) > 0 || abs(camera_motion.z) > 0)
			current_speed = min(1.0f, current_speed + (delta_time * 2.0f));
		else
			current_speed = max(0.0f, current_speed - (delta_time * 2.0f));

		float real_speed = (GetAsyncKeyState(VK_SHIFT) & 0xF000 ? 8.0f : 4.0f) * current_speed;

		//FLOAT up_down = (float)(((GetAsyncKeyState(VK_UP) & 0xF000) > 0) - ((GetAsyncKeyState(VK_DOWN) & 0xF000) > 0));
		//FLOAT left_right = (float)(((GetAsyncKeyState(VK_LEFT) & 0xF000) > 0) - ((GetAsyncKeyState(VK_RIGHT) & 0xF000) > 0));
		FLOAT up_down = mouse_delta.y * 40.0f;
		FLOAT left_right = mouse_delta.x * -40.0f;

		active_camera->transform.rotate(XMFLOAT3(0, 0, 1), left_right * 60.0f * delta_time, active_camera->transform.getPosition());
		active_camera->transform.rotate(active_camera->transform.getRight(), up_down * 60.0f * delta_time, active_camera->transform.getPosition());

		XMFLOAT3 delta;
		if (fly_mode)
		{
			XMStoreFloat3
			(
				&delta,
				XMVector4Transform
				(
					XMVector4Normalize(XMLoadFloat4(&camera_motion)) * delta_time * real_speed,
					XMLoadFloat4x4(&camera_transform)
				)
			);

			selectUnderMouse();
		}
		else
		{
			XMFLOAT3 right = active_camera->transform.getRight();
			camera_motion.y = 0;
			XMStoreFloat4(&camera_motion, XMVector3Normalize(XMLoadFloat4(&camera_motion)));
			XMStoreFloat3
			(
				&delta,
				((XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&right), XMVectorSet(0, 0, 1, 0))) * camera_motion.z) +
				(XMLoadFloat3(&right) * camera_motion.x)) * delta_time * real_speed
			);
		}
		active_camera->transform.translate(delta);
	}
}

//#define VIRTUAL_MACHINE_DEV_ENV
XMFLOAT2 SurrealDemoScene::getMouseDeltaAndReset()
{
#ifdef VIRTUAL_MACHINE_DEV_ENV
	static XMFLOAT2 last_cursor_pos = XMFLOAT2(0,0);
#endif

	POINT cursor;
	GetCursorPos(&cursor);
	XMFLOAT2 window_center = XMFLOAT2(owner->getX() + (owner->getWidth() / 2.0f), owner->getY() + (owner->getHeight() / 2.0f));
	XMFLOAT2 clip_pos = XMFLOAT2(cursor.x - window_center.x, window_center.y - cursor.y);

#ifdef VIRTUAL_MACHINE_DEV_ENV
	XMFLOAT2 delta = XMFLOAT2(clip_pos.x - last_cursor_pos.x, clip_pos.y - last_cursor_pos.y);
	last_cursor_pos = clip_pos;
	return XMFLOAT2(delta.x / owner->getWidth(), delta.y / owner->getWidth());
#else
	if (owner->isFocused())
		SetCursorPos(window_center.x, window_center.y);
	return XMFLOAT2(clip_pos.x / owner->getWidth(), clip_pos.y / owner->getWidth());
#endif
}

void SurrealDemoScene::selectUnderMouse()
{
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
