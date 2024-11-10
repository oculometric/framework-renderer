#include "ShadowDemoScene.h"

#include <windows.h>
#include <iostream>

#include "FResourceManager.h"
#include "FApplication.h"
#include "FDebug.h"
#include "FGraphicsEngine.h"

void ShadowDemoScene::start()
{
	owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SCENE_COLOUR;
}

void ShadowDemoScene::update(float delta_time)
{
	FObject* spinner = findObjectWithName<FObject>("spinner");
	spinner->transform.rotate(XMFLOAT3(0,0,1), delta_time * 30.0f, XMFLOAT3(0,0,0));

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
	}
}
