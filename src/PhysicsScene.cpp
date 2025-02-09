#include "PhysicsScene.h"

#include <DirectXMath.h>
#include <Windows.h>

#include "FDebug.h"
#include "FGraphicsEngine.h"
#include "FPhysicsComponent.h"
#include "FRigidBodyPhysicsComponent.h"
#include "FConstrainedParticleSystemComponent.h"
#include "FAABBCollider.h"
#include "FSphereCollider.h"

using namespace DirectX;

void PhysicsScene::start()
{
	cube_a = findObjectWithName("cube_1");
	cube_b = findObjectWithName("cube_2");

	FObject* obj = new FObject();
	auto comp = new FConstrainedParticleSystemComponent(obj);
	obj->addComponent(comp);
	comp->modulus = 160.0f;
	comp->vertex_mass = 2.0f;
	comp->addVertex(FVector{ 0, 5, 3 }, true);
	comp->addVertex(FVector{ 1, 5, 2 }, false);
	comp->addVertex(FVector{ 3, 5, 2 }, false);
	comp->addVertex(FVector{ 1, 5, 3 }, true);
	comp->addEdge(0, 1, 1.0f);
	comp->addEdge(1, 2, 1.0f);
	comp->addEdge(2, 3, 0.3f);

	addObject(obj, nullptr);

	owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SHARPENED;
	owner->getEngine()->draw_gizmos = true;
}

void PhysicsScene::update(float delta_time)
{
	if (!FApplication::get()->isFocused()) return;

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

	selectUnderMouse();

	if (!active_object) { FDebug::get()->setExtraInfo({}); return; }

	vector<string> info = { active_object->name, str(active_object->transform.getPosition()) };

	XMFLOAT3 object_force = XMFLOAT3
	(
		(float)(((GetAsyncKeyState('L') & 0xF000) > 0) - ((GetAsyncKeyState('J') & 0xF000) > 0)) * delta_time * 4.0f,
		(float)(((GetAsyncKeyState('I') & 0xF000) > 0) - ((GetAsyncKeyState('K') & 0xF000) > 0)) * delta_time * 4.0f,
		(float)(((GetAsyncKeyState('O') & 0xF000) > 0) - ((GetAsyncKeyState('U') & 0xF000) > 0)) * delta_time * 4.0f
	);
	FPhysicsComponent* comp = active_object->getComponent<FPhysicsComponent>();
	if (comp)
	{
		info.push_back(string(comp->obeys_gravity ? "grav on" : "grav off") + " | " + string(comp->kinematic ? "kinematic" : "dynamic"));
		info.push_back(str(comp->getVelocity()));
		info.push_back(format("{:2f}", comp->getMass()));

		if (GetAsyncKeyState('N') & 0x0001)
			comp->obeys_gravity = !comp->obeys_gravity;

		if (GetAsyncKeyState('M') & 0x0001)
			comp->kinematic = !comp->kinematic;

		if (GetAsyncKeyState('B') & 0x0001)
			comp->setVelocity(FVector(0,0,0));

		comp->addForce(object_force * comp->getMass() * speed * 2.0f);
	}
	FDebug::get()->setExtraInfo(info);
}
