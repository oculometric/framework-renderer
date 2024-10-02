#include "MyScene.h"

void MyScene::start()
{
	b.position.x = 1.5f;
	b.scale = XMFLOAT3(0.5f, 0.5f, 0.5f);
	b.eulers.z = 45.0f;
	addObject(&a, nullptr);
	addObject(&b, &a);

	active_camera = new FCamera();
	addObject(active_camera, nullptr);
}

void MyScene::update(float delta_time)
{
	a.eulers.z += 0.8f * delta_time;
	b.eulers.z -= 2.4f * delta_time;
	a.updateTransform();

	// TODO: camera movement
}
