#include "PlanetScene.h"

#include <windows.h>

void PlanetScene::start()
{
	ship_root = findObjectWithName<FObject>("ship_root");
}

void PlanetScene::update(float delta_time)
{
	XMFLOAT4X4 ship_transform = ship_root->getTransform();
	XMFLOAT3 ship_forward = XMFLOAT3(ship_transform._31, ship_transform._32, ship_transform._33);
	float dist = delta_time * speed;
	XMFLOAT3 pos = ship_root->position;
	ship_root->position = XMFLOAT3(ship_forward.x * dist + pos.x, ship_forward.y * dist + pos.y, ship_forward.z * dist + pos.z);

	if (GetAsyncKeyState('W') & 0xF000) ship_root->eulers.x -= delta_time * 60.0f;
	if (GetAsyncKeyState('S') & 0xF000) ship_root->eulers.x += delta_time * 60.0f;
	if (GetAsyncKeyState('A') & 0xF000) ship_root->eulers.z -= delta_time * 60.0f;
	if (GetAsyncKeyState('D') & 0xF000) ship_root->eulers.z += delta_time * 60.0f;

	// TODO: improve transformability of FObject - rotate about vector, matrix->components, set/get for components

	if (GetAsyncKeyState('E')) speed += delta_time * 2.0f;
	if (GetAsyncKeyState('Q')) speed -= delta_time * 2.0f;

	ship_root->updateTransform();
}
