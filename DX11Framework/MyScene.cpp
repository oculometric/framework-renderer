#include "MyScene.h"

#include <windows.h>
#include <iostream>
#include "FApplication.h"
#include "ico.h"

const float s3 = 1.0f / sqrtf(3.0f);

vector<FVertex> cube_vertices =
{
	FVertex{ XMFLOAT3(1.0f,  1.0f, 1.0f),   XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(s3, s3, s3),   },
	FVertex{ XMFLOAT3(-1.0f,  1.0f, 1.0f),  XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(-s3, s3, s3),  },
	FVertex{ XMFLOAT3(1.0f, -1.0f, 1.0f),   XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(s3, -s3, s3),  },
	FVertex{ XMFLOAT3(-1.0f, -1.0f, 1.0f),  XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(-s3, -s3, s3), },
	FVertex{ XMFLOAT3(1.0f,  1.0f, -1.0f),  XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT3(s3, s3, -s3),  },
	FVertex{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT3(-s3, s3, -s3), },
	FVertex{ XMFLOAT3(1.0f, -1.0f, -1.0f),  XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(s3, -s3, -s3), },
	FVertex{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-s3, -s3, -s3) },
};

vector<uint16_t> cube_indices =
{
	0, 1, 2,
	1, 3, 2,

	2, 3, 7,
	2, 7, 6,

	1, 7, 3,
	1, 5, 7,

	0, 2, 6,
	0, 6, 4,

	1, 0, 5,
	0, 4, 5,

	5, 6, 7,
	4, 6, 5
};

void MyScene::start()
{
	FMeshData* cube_mesh = new FMeshData();
	cube_mesh->vertices = cube_vertices;
	cube_mesh->indices = cube_indices;
	owner->registerMesh(cube_mesh);

	FMeshData* ico = new FMeshData();
	ico->vertices = ico_vertices;
	ico->indices = ico_indices;
	owner->registerMesh(ico);

	FMeshData* suzanne = FMesh::loadMesh("suzanne.obj");
	owner->registerMesh(suzanne);

	FMeshData* teapot = FMesh::loadMesh("teapot.obj");
	owner->registerMesh(teapot);

	FMeshData* sphere = FMesh::loadMesh("sphere.obj");
	owner->registerMesh(sphere);

	FMeshData* cornell = FMesh::loadMesh("cornell.obj");
	owner->registerMesh(cornell);

	FMeshData* monitor = FMesh::loadMesh("monitor.obj");
	owner->registerMesh(monitor);
	FMaterial* monitor_mat = new FMaterial();
	monitor_mat->textures[0] = owner->registerTexture(L"monitor_t.dds");
	monitor_mat->textures[1] = owner->registerTexture(L"monitor_n.dds");

	a.setData(monitor);
	a.setMaterial(monitor_mat);
	b.setData(suzanne);
	c.setData(teapot);

	b.position.x = 4.5f;
	b.scale = XMFLOAT3(0.3f, 0.3f, 0.3f);
	b.eulers.z = 45.0f;
	addObject(&a, nullptr);
	addObject(&b, &a);

	c.position = XMFLOAT3(0.0f, 0.0f, 2.0f);
	addObject(&c, nullptr);

	active_camera = new FCamera();
	active_camera->position.z = 5.0f;
	active_camera->eulers.y = 180.0f;
	active_camera->updateProjectionMatrix();
	addObject(active_camera, nullptr);

	FMesh* box = new FMesh();
	box->position.z = 6.0f;
	box->setData(cornell);
	addObject(box, nullptr);
}

void MyScene::update(float delta_time)
{
	a.eulers.z += 24.0f * delta_time;
	b.eulers.z -= 180.0f * delta_time;
	b.eulers.y += 36.0f * delta_time;

	c.eulers.x += 24.0f * delta_time;
	c.eulers.y += 36.0f * delta_time;
	c.eulers.z += 12.0f * delta_time;

	root.updateTransform();
	
	if (owner->isFocused())
	{
		XMFLOAT4X4 camera_transform = active_camera->getTransform();
		XMFLOAT4 camera_motion = XMFLOAT4
		(
			(float)((GetAsyncKeyState('D') & 0xF000) - (GetAsyncKeyState('A') & 0xF000)),
			(float)((GetAsyncKeyState('E') & 0xF000) - (GetAsyncKeyState('Q') & 0xF000)),
			(float)((GetAsyncKeyState('W') & 0xF000) - (GetAsyncKeyState('S') & 0xF000)),
			0.0f
		);

		FLOAT speed = GetAsyncKeyState(VK_SHIFT) & 0xF000 ? 8.0f : 3.0f;

		if (GetAsyncKeyState(VK_UP) & 0xF000) active_camera->eulers.x += delta_time * 60.0f;
		if (GetAsyncKeyState(VK_DOWN) & 0xF000) active_camera->eulers.x -= delta_time * 60.0f;
		if (GetAsyncKeyState(VK_LEFT) & 0xF000) active_camera->eulers.z -= delta_time * 60.0f;
		if (GetAsyncKeyState(VK_RIGHT) & 0xF000) active_camera->eulers.z += delta_time * 60.0f;

		XMStoreFloat3
		(
			&active_camera->position,
			XMVector4Transform
			(
				XMVector4Normalize(XMLoadFloat4(&camera_motion)) * delta_time * speed,
				XMLoadFloat4x4(&camera_transform)
			) + XMLoadFloat3(&active_camera->position)
		);
		active_camera->updateTransform();
	}
}
