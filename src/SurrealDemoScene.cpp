#include "SurrealDemoScene.h"

#include <windows.h>
#include <iostream>

#include "FResourceManager.h"
#include "FApplication.h"
#include "FDebug.h"
#include "FGraphicsEngine.h"

void SurrealDemoScene::start()
{
	fly_cam = findObjectWithName<FCamera>("fly_cam");
	walk_cam = findObjectWithName<FCamera>("walk_cam");
	active_camera = fly_mode ? fly_cam : walk_cam;

	getMouseDeltaAndReset();
}

void SurrealDemoScene::update(float delta_time)
{
	XMFLOAT2 mouse_delta = getMouseDeltaAndReset();

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

		FLOAT speed = GetAsyncKeyState(VK_SHIFT) & 0xF000 ? 8.0f : 3.0f;

		//FLOAT up_down = (float)(((GetAsyncKeyState(VK_UP) & 0xF000) > 0) - ((GetAsyncKeyState(VK_DOWN) & 0xF000) > 0));
		//FLOAT left_right = (float)(((GetAsyncKeyState(VK_LEFT) & 0xF000) > 0) - ((GetAsyncKeyState(VK_RIGHT) & 0xF000) > 0));
		FLOAT up_down = mouse_delta.y / 1.0f;
		FLOAT left_right = mouse_delta.x / -1.0f;

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
					XMVector4Normalize(XMLoadFloat4(&camera_motion)) * delta_time * speed,
					XMLoadFloat4x4(&camera_transform)
				)
			);
		}
		else
		{
			XMFLOAT4 quat = active_camera->transform.getQuaternion();
			//quat.y = max(min(quat.y, 0.97f), 0.03f);
			//XMStoreFloat4(&quat, XMVector3Normalize(XMLoadFloat4(&quat)));
			active_camera->transform.setQuaternion(quat);

			XMFLOAT3 right = active_camera->transform.getRight();
			camera_motion.y = 0;
			XMStoreFloat4(&camera_motion, XMVector3Normalize(XMLoadFloat4(&camera_motion)));
			XMStoreFloat3
			(
				&delta,
				((XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&right), XMVectorSet(0, 0, 1, 0))) * camera_motion.z) +
				(XMLoadFloat3(&right) * camera_motion.x)) * delta_time * speed
			);
		}
		active_camera->transform.translate(delta);
	}
}

#define VIRTUAL_MACHINE_DEV_ENV
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
	return delta;
#else
	if (owner->isFocused())
		SetCursorPos(window_center.x, window_center.y);
	return clip_pos;
#endif
}
