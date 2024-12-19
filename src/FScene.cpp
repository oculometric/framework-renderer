#include "FScene.h"

#include "FMesh.h"
#include "FResourceManager.h"
#include "FGraphicsEngine.h"
#include "FDeserialiser.h"

using namespace std;

FScene::FScene(FApplication* application, string scene_file)
{
	owner = application;
	if (!scene_file.empty()) deserialiseScene(FJsonBlob(scene_file).getRoot(), this);
}

void FScene::addObject(FObject* o, FObject* parent)
{
	if (o == nullptr) return;
	if (all_objects.count(o) > 0) return;

	if (parent == nullptr)
	{
		all_objects.insert(o);
		root.transform.addChild(&(o->transform));
	}
	else
	{
		parent->transform.addChild(&(o->transform));
		all_objects.insert(o);
	}

	FLight* l = o->getComponent<FLight>();
	if (l != nullptr) all_lights.insert(l);

	FCameraComponent* c = o->getComponent<FCameraComponent>();
	if (c != nullptr) active_camera = c;
}

FObject* FScene::findObjectWithName(std::string str)
{
	for (FObject* obj : all_objects)
		if (obj->name == str)
			return obj;

	return nullptr;
}

void FScene::selectUnderMouse()
{
	if (GetAsyncKeyState(VK_LBUTTON) & 0xF000)
	{
		POINT p;
		GetCursorPos(&p);
		RECT r;
		GetWindowRect(owner->getWindow(), &r);
		XMFLOAT2 screen_pos = XMFLOAT2((float)(p.x - r.left) / (float)(r.right - r.left), (float)(p.y - r.top) / (float)(r.bottom - r.top));
		XMFLOAT4 clip_pos = XMFLOAT4(screen_pos.x * 2.0f - 1.0f, (screen_pos.y * 2.0f - 1.0f) * -1.0f, 0, 1);
		XMFLOAT4X4 proj = active_camera->getProjectionMatrix();
		XMFLOAT4X4 view = active_camera->getOwner()->transform.getTransform();
		XMFLOAT3 world_direction;
		XMStoreFloat3
		(
			&world_direction,
			XMVector3Normalize
			(
				XMVector4Transform
				(
					XMVector4Transform
					(
						XMLoadFloat4(&clip_pos),
						XMMatrixInverse(nullptr, XMLoadFloat4x4(&proj))
					) * XMVectorSet(1, 1, 1, 0),
					XMLoadFloat4x4(&view)
				)
			)
		);

		XMFLOAT3 origin = XMFLOAT3(view._41, view._42, view._43);

		FObject* closest = nullptr;
		float dist = INFINITY;

		for (FObject* obj : all_objects)
		{
			FMesh* m = obj->getComponent<FMesh>();
			if (m == nullptr) continue;

			FBoundingBox box = m->getWorldSpaceBounds();

			float tmin;
			float tmax;

			if (FMesh::intersectBoundingBox(box, origin, world_direction, tmin, tmax) && tmin < dist)
			{
				closest = obj;
				dist = tmin;
			}
		}

		active_object = closest;
	}
}
