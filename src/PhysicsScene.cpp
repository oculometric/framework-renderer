#include "PhysicsScene.h"

#include <DirectXMath.h>
#include <Windows.h>

#include "FDebug.h"
#include "FGraphicsEngine.h"
#include "FPhysicsComponent.h"
#include "FRigidBodyPhysicsComponent.h"
#include "FAABBCollider.h"

using namespace DirectX;

void PhysicsScene::start()
{
	cube_a = findObjectWithName("cube_1");
	cube_b = findObjectWithName("cube_2");

	FRigidBodyPhysicsComponent* comp_a = cube_a->getComponent<FRigidBodyPhysicsComponent>();
	FAABBCollider* coll_a = new FAABBCollider(comp_a);
	coll_a->setBounds(FBoundingBox(FVector(0, 0, 0), FVector(1, 1, 1)));
	comp_a->setCollider(coll_a);

	FObject* plane = findObjectWithName("plane");
	FRigidBodyPhysicsComponent* comp_p = plane->getComponent<FRigidBodyPhysicsComponent>();
	FAABBCollider* coll_p = new FAABBCollider(comp_p);
	coll_p->setBounds(FBoundingBox(FVector(0, 0, -0.5), FVector(12, 12, 1)));
	comp_p->setCollider(coll_p);

	owner->getEngine()->output_mode = FGraphicsEngine::FOutputMode::SHARPENED;
	owner->getEngine()->draw_gizmos = true;
}

void PhysicsScene::update(float delta_time)
{
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

	if (!active_object) return;

	XMFLOAT3 object_force = XMFLOAT3
	(
		(float)(((GetAsyncKeyState('L') & 0xF000) > 0) - ((GetAsyncKeyState('J') & 0xF000) > 0)) * delta_time * 4.0f,
		(float)(((GetAsyncKeyState('I') & 0xF000) > 0) - ((GetAsyncKeyState('K') & 0xF000) > 0)) * delta_time * 4.0f,
		(float)(((GetAsyncKeyState('O') & 0xF000) > 0) - ((GetAsyncKeyState('U') & 0xF000) > 0)) * delta_time * 4.0f
	);
	FPhysicsComponent* comp = active_object->getComponent<FPhysicsComponent>();
	if (comp)
	{
		comp->addForce(object_force);
	}
}
