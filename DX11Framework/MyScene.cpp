#include "MyScene.h"

#include <windows.h>
#include <iostream>
#include "FApplication.h"
#include "ico.h"
#include "../oculib/mesh.h"

XMFLOAT3 vertex_positions[] =
{
	XMFLOAT3(1.0f,  1.0f, 1.0f),
	XMFLOAT3(-1.0f,  1.0f, 1.0f),
	XMFLOAT3(1.0f, -1.0f, 1.0f),
	XMFLOAT3(-1.0f, -1.0f, 1.0f),
	XMFLOAT3(1.0f,  1.0f, -1.0f),
	XMFLOAT3(-1.0f,  1.0f, -1.0f),
	XMFLOAT3(1.0f, -1.0f, -1.0f),
	XMFLOAT3(-1.0f, -1.0f, -1.0f)
};

XMFLOAT4 vertex_colours[] =
{
	XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f),
	XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),
	XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
	XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),
	XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
	XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
	XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f)
};

const float s3 = 1.0f / sqrtf(3.0);

XMFLOAT3 vertex_normals[] =
{
	XMFLOAT3(s3, s3, s3),
	XMFLOAT3(-s3, s3, s3),
	XMFLOAT3(s3, -s3, s3),
	XMFLOAT3(-s3, -s3, s3),
	XMFLOAT3(s3, s3, -s3),
	XMFLOAT3(-s3, s3, -s3),
	XMFLOAT3(s3, -s3, -s3),
	XMFLOAT3(-s3, -s3, -s3)
};

uint16_t indices[] =
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
	FMeshData* data = new FMeshData();
	data->position = vertex_positions;
	data->colour = vertex_colours;
	data->normal = vertex_normals;
	data->indices = indices;
	data->vertex_count = 8;
	data->index_count = 36;
	owner->registerMesh(data);

	FMeshData* ico = new FMeshData();
	ico->position = ico_vertex_positions;
	ico->colour = ico_vertex_colours;
	ico->normal = new XMFLOAT3[12];
	ico->indices = ico_indices;
	ico->vertex_count = 12;
	ico->index_count = 60;
	owner->registerMesh(ico);

	OLMesh suzanne = OLMesh("suzanne.obj");
	FMeshData* monkey = new FMeshData();
	monkey->position = new XMFLOAT3[suzanne.vertices.size()];
	for (int i = 0; i < suzanne.vertices.size(); i++) 
		monkey->position[i] = XMFLOAT3
		(
			suzanne.vertices[i].position.x,
			suzanne.vertices[i].position.y,
			suzanne.vertices[i].position.z
		);
	monkey->colour = new XMFLOAT4[suzanne.vertices.size()];
	monkey->normal = new XMFLOAT3[suzanne.vertices.size()];
	monkey->indices = new uint16_t[suzanne.indices.size()];
	memcpy(monkey->indices, suzanne.indices.data(), suzanne.indices.size() * sizeof(monkey->indices[0]));
	monkey->vertex_count = suzanne.vertices.size();
	monkey->index_count = suzanne.indices.size();
	owner->registerMesh(monkey);

	a.setData(data);
	b.setData(data);

	b.position.x = 4.5f;
	b.scale = XMFLOAT3(0.3f, 0.3f, 0.3f);
	b.eulers.z = 45.0f;
	addObject(&a, nullptr);
	addObject(&b, &a);

	active_camera = new FCamera();
	active_camera->position.z = 5.0f;
	active_camera->eulers.y = 180.0f;
	active_camera->updateProjectionMatrix();
	addObject(active_camera, nullptr);
}

void MyScene::update(float delta_time)
{
	a.eulers.z += 24.0f * delta_time;
	b.eulers.z -= 180.0f * delta_time;
	b.eulers.y += 36.0f * delta_time;
	a.updateTransform();
	
	XMFLOAT4X4 camera_transform = active_camera->getTransform();
	XMFLOAT4 camera_motion = 
	XMFLOAT4(
		(GetAsyncKeyState('D') & 0xF000) - (GetAsyncKeyState('A') & 0xF000),
		(GetAsyncKeyState('E') & 0xF000) - (GetAsyncKeyState('Q') & 0xF000),
		(GetAsyncKeyState('W') & 0xF000) - (GetAsyncKeyState('S') & 0xF000),
		0
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
