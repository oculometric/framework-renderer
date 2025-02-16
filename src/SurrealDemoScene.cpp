#include "SurrealDemoScene.h"

#include <windows.h>
#include <iostream>

#include "FResourceManager.h"
#include "FApplication.h"
#include "FDebug.h"
#include "FGraphicsEngine.h"
#include "FComponent.h"

using namespace std;

void SurrealDemoScene::start()
{
	orrery_base = findObjectWithName("orrery_base");
	orrery_mid = findObjectWithName("orrery_mid");
	orrery_planet_a = findObjectWithName("orrery_planet_a");
	orrery_planet_b = findObjectWithName("orrery_planet_b");
	orrery_core = findObjectWithName("orrery_core");

	monitor = findObjectWithName("monitor");

	fly_cam = findObjectWithName("fly_cam");
	walk_cam = findObjectWithName("walk_cam");
	active_camera = (fly_mode ? fly_cam : walk_cam)->getComponent<FCameraComponent>();

	getMouseDeltaAndReset();
}

void SurrealDemoScene::update(float delta_time)
{
	XMFLOAT2 mouse_delta = getMouseDeltaAndReset();

	orrery_mid->transform.rotate(orrery_mid->transform.getRight(), delta_time * 30.0f, orrery_mid->transform.getPosition());
	orrery_core->transform.rotate(orrery_core->transform.getForward(), delta_time * 45.0f, orrery_core->transform.getPosition());
	orrery_planet_a->transform.rotate(orrery_planet_a->transform.getForward(), delta_time * 10.0f, orrery_planet_a->transform.getPosition());
	orrery_planet_b->transform.rotate(orrery_planet_b->transform.getForward(), delta_time * 2.0f, orrery_planet_b->transform.getPosition());
	monitor->transform.rotate(XMFLOAT3(0, 0, 1), delta_time * 30.0f, monitor->transform.getPosition());

	if (owner->isFocused() && active_camera != nullptr)
	{
		if (GetAsyncKeyState('1') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::POST_PROCESS;
		if (GetAsyncKeyState('2') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SCENE_COLOUR;
		if (GetAsyncKeyState('3') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SCENE_NORMAL;
		if (GetAsyncKeyState('4') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SCENE_DEPTH;
		if (GetAsyncKeyState('5') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SHARPENED;
		if (GetAsyncKeyState('6') & 0xF000) owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::AMBIENT_OCCLUSION;
		if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
		{
			owner->clearFocus();
			return;
		}

		if (interaction_mode == 0)
		{
			if (GetAsyncKeyState('V') & 0x0001)
				owner->getEngine()->enable_vsync = !(owner->getEngine()->enable_vsync);

			if (GetAsyncKeyState(VK_TAB) & 0x0001)
			{
				fly_mode = !fly_mode;
				owner->getEngine()->draw_gizmos = fly_mode;
				active_camera = (fly_mode ? fly_cam : walk_cam)->getComponent<FCameraComponent>();
			}

			// if in fly mode, and an object is selected, we can enter transform modes
			if (fly_mode && active_object != nullptr)
			{
				if (GetAsyncKeyState('G') & 0x0001)
				{
					interaction_mode = 1;
					original_transform = active_object->transform.getTransform();
					return;
				}
				if (GetAsyncKeyState('R') & 0x0001)
				{
					interaction_mode = 2;
					original_transform = active_object->transform.getTransform();
					return;
				}
				if (GetAsyncKeyState('F') & 0x0001)
				{
					interaction_mode = 3;
					original_transform = active_object->transform.getTransform();
					return;
				}
			}

			XMFLOAT4X4 camera_transform = active_camera->getOwner()->transform.getTransform();
			// camera-local motion vector based on input, which we will transform into world space using the camera's transform
			XMFLOAT4 camera_motion = XMFLOAT4
			(
				(float)((GetAsyncKeyState('D') & 0xF000) - (GetAsyncKeyState('A') & 0xF000)),
				(float)((GetAsyncKeyState('E') & 0xF000) - (GetAsyncKeyState('Q') & 0xF000)),
				(float)((GetAsyncKeyState('S') & 0xF000) - (GetAsyncKeyState('W') & 0xF000)),
				0.0f
			);

			// camera acceleration, to smooth things out
			if (abs(camera_motion.x) > 0 || abs(camera_motion.y) > 0 || abs(camera_motion.z) > 0)
				current_speed = min(1.0f, current_speed + (delta_time * 2.0f));
			else
				current_speed = max(0.0f, current_speed - (delta_time * 2.0f));

			float real_speed = (GetAsyncKeyState(VK_SHIFT) & 0xF000 ? 8.0f : 4.0f) * current_speed;

			FLOAT up_down = mouse_delta.y * 80.0f;
			FLOAT left_right = mouse_delta.x * -80.0f;

			active_camera->getOwner()->transform.rotate(XMFLOAT3(0, 0, 1), left_right * 60.0f * delta_time, active_camera->getOwner()->transform.getPosition());
			active_camera->getOwner()->transform.rotate(active_camera->getOwner()->transform.getRight(), up_down * 60.0f * delta_time, active_camera->getOwner()->transform.getPosition());

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
				XMFLOAT3 right = active_camera->getOwner()->transform.getRight();
				camera_motion.y = 0;
				XMStoreFloat4(&camera_motion, XMVector3Normalize(XMLoadFloat4(&camera_motion)));
				XMStoreFloat3
				(
					&delta,
					((XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&right), XMVectorSet(0, 0, 1, 0))) * camera_motion.z) +
					(XMLoadFloat3(&right) * camera_motion.x)) * delta_time * real_speed
				);
			}
			active_camera->getOwner()->transform.translate(delta);
		}
		else if (interaction_mode == 1)
		{
			// user can move the mouse to move the selected object
			if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
			{
				active_object->transform.setTransform(original_transform);
				interaction_mode = 0;
			}
			else if (GetAsyncKeyState(VK_LBUTTON) & 0xF000)
			{
				interaction_mode = 0;
			}

			XMFLOAT3 x = active_camera->getOwner()->transform.getRight();
			XMFLOAT3 y = active_camera->getOwner()->transform.getUp();
			XMFLOAT3 direction = XMFLOAT3((x.x * mouse_delta.x) + (y.x * mouse_delta.y), (x.y * mouse_delta.x) + (y.y * mouse_delta.y), (x.z * mouse_delta.x) + (y.z * mouse_delta.y));
			direction = XMFLOAT3(direction.x * 4.0f, direction.y * 4.0f, direction.z * 4.0f);
			active_object->transform.translate(direction);
		}
		else if (interaction_mode == 2)
		{
			// user can rotate object about the view axis by moving the mouse horizontally
			if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
			{
				active_object->transform.setTransform(original_transform);
				interaction_mode = 0;
			}
			else if (GetAsyncKeyState(VK_LBUTTON) & 0xF000)
			{
				interaction_mode = 0;
			}

			active_object->transform.rotate(active_camera->getOwner()->transform.getForward(), mouse_delta.x * -90.0f, active_object->transform.getPosition());
		}
		else if (interaction_mode == 3)
		{
			// user can scale the object by moving the mouse
			if (GetAsyncKeyState(VK_ESCAPE) & 0x0001)
			{
				active_object->transform.setTransform(original_transform);
				interaction_mode = 0;
			}
			else if (GetAsyncKeyState(VK_LBUTTON) & 0xF000)
			{
				interaction_mode = 0;
			}
			
			float delta = 1.0f + mouse_delta.x + mouse_delta.y;
			active_object->transform.scale(XMFLOAT3(delta, delta, delta), active_object->transform.getPosition());
		}
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
	XMFLOAT2 window_center = XMFLOAT2(floor(owner->getX() + (owner->getWidth() / 2.0f)), floor(owner->getY() + (owner->getHeight() / 2.0f)));
	XMFLOAT2 clip_pos = XMFLOAT2(cursor.x - window_center.x, window_center.y - cursor.y);

#ifdef VIRTUAL_MACHINE_DEV_ENV
	XMFLOAT2 delta = XMFLOAT2(clip_pos.x - last_cursor_pos.x, clip_pos.y - last_cursor_pos.y);
	last_cursor_pos = clip_pos;
	return XMFLOAT2(delta.x / (owner->getWidth() / 2), delta.y / (owner->getWidth() / 2));
#else
	if (owner->isFocused())
		SetCursorPos((int)window_center.x, (int)window_center.y);
	return XMFLOAT2(clip_pos.x / owner->getWidth(), clip_pos.y / owner->getWidth());
#endif
}

