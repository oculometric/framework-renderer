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
}

void SurrealDemoScene::update(float delta_time)
{
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

		FLOAT up_down = (float)(((GetAsyncKeyState(VK_UP) & 0xF000) > 0) - ((GetAsyncKeyState(VK_DOWN) & 0xF000) > 0));
		FLOAT left_right = (float)(((GetAsyncKeyState(VK_LEFT) & 0xF000) > 0) - ((GetAsyncKeyState(VK_RIGHT) & 0xF000) > 0));

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
