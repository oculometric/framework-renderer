#include "PlanetScene.h"

#include <windows.h>
#include "FMesh.h"

void PlanetScene::start()
{
	ship_root = findObjectWithName("ship_root");
	sun = findObjectWithName("sun");
}

void PlanetScene::update(float delta_time)
{
	XMFLOAT3 forward = ship_root->transform.getForward();
	forward = XMFLOAT3(-forward.x * delta_time * speed, -forward.y * delta_time * speed, -forward.z * delta_time * speed);
	ship_root->transform.translate(forward);

	if (GetAsyncKeyState('W') & 0xF000) ship_root->transform.rotate(ship_root->transform.getRight(), -delta_time * 60.0f, ship_root->transform.getPosition());
	if (GetAsyncKeyState('S') & 0xF000) ship_root->transform.rotate(ship_root->transform.getRight(), delta_time * 60.0f, ship_root->transform.getPosition());
	if (GetAsyncKeyState('A') & 0xF000) ship_root->transform.rotate(ship_root->transform.getForward(), delta_time * 60.0f, ship_root->transform.getPosition());
	if (GetAsyncKeyState('D') & 0xF000) ship_root->transform.rotate(ship_root->transform.getForward(), -delta_time * 60.0f, ship_root->transform.getPosition());

	if (GetAsyncKeyState('E')) speed += delta_time * 2.0f;
	if (GetAsyncKeyState('Q')) speed -= delta_time * 2.0f;

	sun->transform.rotate(XMFLOAT3(0,0,1), delta_time, XMFLOAT3(0,0,0));
}
