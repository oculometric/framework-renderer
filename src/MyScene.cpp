#include "MyScene.h"

#include <windows.h>
#include <iostream>

#include "FResourceManager.h"
#include "FApplication.h"
#include "FDebug.h"
#include "FGraphicsEngine.h"

using namespace std;

void MyScene::start()
{
	// 99% of the initialisation is now done from the config file! see MyScene.json
	a = findObjectWithName("a");
	b = findObjectWithName("sphere");
	c = findObjectWithName("c");
}

void MyScene::update(float delta_time)
{
	b->transform.rotate(XMFLOAT3(0, 0, 1), 180.0f * delta_time, b->transform.getPosition());
	c->transform.rotate(XMFLOAT3(1, 0, 0), 24.0f * delta_time, c->transform.getPosition());
	c->transform.rotate(XMFLOAT3(0, 1, 0), 36.0f * delta_time, c->transform.getPosition());
	c->transform.rotate(XMFLOAT3(0, 0, 1), 12.0f * delta_time, c->transform.getPosition());
	
	if (owner->isFocused() && active_camera != nullptr)
	{
		if (GetAsyncKeyState('1') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::POST_PROCESS;
		if (GetAsyncKeyState('2') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SCENE_COLOUR;
		if (GetAsyncKeyState('3') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SCENE_NORMAL;
		if (GetAsyncKeyState('4') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SCENE_DEPTH;
		if (GetAsyncKeyState('5') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SHARPENED;

		FDebug::console(to_string(owner->getEngine()->output_mode));

		XMFLOAT4X4 camera_transform = active_camera->getOwner()->transform.getTransform();
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

		active_camera->getOwner()->transform.rotate(XMFLOAT3(0, 0, 1), left_right * 60.0f * delta_time, active_camera->getOwner()->transform.getPosition());
		active_camera->getOwner()->transform.rotate(active_camera->getOwner()->transform.getRight(), up_down * 60.0f * delta_time, active_camera->getOwner()->transform.getPosition());

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
		active_camera->getOwner()->transform.translate(delta);

		if (active_object)
		{
			XMFLOAT3 object_motion = XMFLOAT3
			(
				(float)(((GetAsyncKeyState('U') & 0xF000) > 0) - ((GetAsyncKeyState('Y') & 0xF000) > 0)) * delta_time,
				(float)(((GetAsyncKeyState('H') & 0xF000) > 0) - ((GetAsyncKeyState('G') & 0xF000) > 0)) * delta_time,
				(float)(((GetAsyncKeyState('B') & 0xF000) > 0) - ((GetAsyncKeyState('V') & 0xF000) > 0)) * delta_time
			);
			active_object->transform.translate(object_motion);
		}

		
	}
}
