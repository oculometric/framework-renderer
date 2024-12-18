#include "FScene.h"

#include "FJsonParser.h"
#include "FMesh.h"
#include "FResourceManager.h"
#include "FGraphicsEngine.h"

using namespace std;

FScene::FScene(FApplication* application, string scene_file)
{
	owner = application;
	if (!scene_file.empty()) FJsonBlob(scene_file).getRoot() >> *this;
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
		if (all_objects.count(parent) == 0) return;
		all_objects.insert(o);
		parent->transform.addChild(&(o->transform));
	}

	// FIXME: convert this to a map/set (ie unique elements)
	FLight* l = o->getComponent<FLight>();
	if (l != nullptr) all_lights.push_back(l);
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

void FScene::finalizePreload()
{
	for (FObjectPreload o : preload_array)
		finalizeObject(o, nullptr);
	preload_array.clear();
}

void FScene::finalizeObject(FObjectPreload& o, FObject* parent)
{
	FResourceManager* rm = FResourceManager::get();

	FObject* obj = new FObject();
	obj->name = o.name;
	obj->transform = FTransform();

	switch (o.object_type)
	{
	case FComponentType::MESH:
	{
		FMesh* me = new FMesh(obj);
		if (o.data_name != "") me->setData(rm->loadMesh(o.data_name));
		if (o.material_name != "") me->setMaterial(rm->getMaterial(o.material_name));
		me->cast_shadow = o.cast_shadow;

		obj->addComponent(me);
		break;
	}
	case FComponentType::CAMERA:
	{
		FCameraComponent* cam = new FCameraComponent(obj);
		if (o.float1 > 0) cam->aspect_ratio = o.float1;
		if (o.float2 > 0) cam->field_of_view = o.float2;
		if (o.float3 > 0) cam->near_clip = o.float3;
		if (o.float4 > 0) cam->far_clip = o.float4;

		cam->updateProjectionMatrix();

		obj->addComponent(cam);
		active_camera = cam;
		break;
	}
	case FComponentType::LIGHT:
	{
		FLight* light = new FLight(obj);
		light->angle = o.angle;
		light->strength = o.strength;
		light->colour = o.colour;
		if (o.data_name == "directional") light->type = FLight::FLightType::DIRECTIONAL;
		else if (o.data_name == "spot") light->type = FLight::FLightType::SPOT;
		else if (o.data_name == "point") light->type = FLight::FLightType::POINT;

		obj->addComponent(light);
		break;
	}
	default:
		break;
	}
	
	addObject(obj, parent);

	obj->transform.setLocalPosition(o.position);
	obj->transform.setLocalEuler(o.rotation);
	obj->transform.setLocalScale(o.scale);
	
	for (FObjectPreload c : o.children)
		finalizeObject(c, obj);
}
