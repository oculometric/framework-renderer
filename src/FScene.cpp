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
		FCamera* cam = new FCamera(obj);
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
		obj = new FObject();
		break;
	}
	
	addObject(obj, parent);

	obj->transform.setLocalPosition(o.position);
	obj->transform.setLocalEuler(o.rotation);
	obj->transform.setLocalScale(o.scale);
	
	for (FObjectPreload c : o.children)
		finalizeObject(c, obj);
}

bool operator>>(const FJsonElement& a, XMFLOAT3& other)
{
	if (a.type != JARRAY) return false;
	if (a.a_val.size() != 3) return false;

	if (a.a_val[0].type == JFLOAT) other.x = a.a_val[0].f_val;
	if (a.a_val[1].type == JFLOAT) other.y = a.a_val[1].f_val;
	if (a.a_val[2].type == JFLOAT) other.z = a.a_val[2].f_val;

	return true;
}

bool operator>>(const FJsonElement& a, XMFLOAT4& other)
{
	if (a.type != JARRAY) return false;
	if (a.a_val.size() != 4) return false;

	if (a.a_val[0].type == JFLOAT) other.x = a.a_val[0].f_val;
	if (a.a_val[1].type == JFLOAT) other.y = a.a_val[1].f_val;
	if (a.a_val[2].type == JFLOAT) other.z = a.a_val[2].f_val;
	if (a.a_val[3].type == JFLOAT) other.w = a.a_val[3].f_val;

	return true;
}

bool operator>>(const FJsonElement& a, XMINT3& other)
{
	if (a.type != JARRAY) return false;
	if (a.a_val.size() != 3) return false;

	if (a.a_val[0].type == JFLOAT) other.x = (INT)a.a_val[0].f_val;
	if (a.a_val[1].type == JFLOAT) other.y = (INT)a.a_val[1].f_val;
	if (a.a_val[2].type == JFLOAT) other.z = (INT)a.a_val[2].f_val;

	return true;
}

bool operator>>(const FJsonElement& a, FObjectPreload& other)
{
	if (a.type != JOBJECT) return false;

	FJsonObject* obj = a.o_val;
	if (obj == nullptr) return false;
	if (!obj->has("class", JSTRING)) return false;
	string object_class = (*obj)["class"].s_val;
	if (object_class == "mesh")
	{
		other.object_type = FComponentType::MESH;
		if (obj->has("data", JSTRING)) other.data_name = (*obj)["data"].s_val;
		if (obj->has("material", JSTRING)) other.material_name = (*obj)["material"].s_val;
		if (obj->has("cast_shadow", JFLOAT)) other.cast_shadow = (*obj)["cast_shadow"].f_val > 0.0f;
	}
	else if (object_class == "camera")
	{
		other.object_type = FComponentType::CAMERA;
		if (obj->has("aspect_ratio", JFLOAT)) other.float1 = (*obj)["aspect_ratio"].f_val;
		if (obj->has("field_of_view", JFLOAT)) other.float2 = (*obj)["field_of_view"].f_val;
		if (obj->has("near_clip", JFLOAT)) other.float3 = (*obj)["near_clip"].f_val;
		if (obj->has("far_clip", JFLOAT)) other.float4 = (*obj)["far_clip"].f_val;
	}
	else if (object_class == "empty")
		other.object_type = FComponentType::BLANK;
	else if (object_class == "light")
	{
		other.object_type = FComponentType::LIGHT;
		if (obj->has("colour", JARRAY)) (*obj)["colour"] >> other.colour;
		if (obj->has("strength", JFLOAT)) other.strength = (*obj)["strength"].f_val;
		if (obj->has("angle", JFLOAT)) other.angle = (*obj)["angle"].f_val;
		if (obj->has("type", JSTRING)) other.data_name = (*obj)["type"].s_val;
	}
	else
		return false;

	if (obj->has("name", JSTRING)) other.name = (*obj)["name"].s_val;
	if (obj->has("position", JARRAY)) (*obj)["position"] >> other.position;
	if (obj->has("rotation", JARRAY)) (*obj)["rotation"] >> other.rotation;
	if (obj->has("scale", JARRAY)) (*obj)["scale"] >> other.scale;

	if (obj->has("children", JARRAY))
	{
		vector<FJsonElement> cs = (*obj)["children"].a_val;
		for (FJsonElement c : cs)
		{
			FObjectPreload fop;
			if (c >> fop)
				other.children.push_back(fop);
		}
	}

	return true;
}

bool operator>>(const FJsonElement& a, FScene& other)
{
	if (a.type != JOBJECT) return false;

	FResourceManager* rm = FResourceManager::get();

	FJsonObject* scene_obj = a.o_val;
	if (scene_obj == nullptr) return false;
	if (scene_obj->has("name", JSTRING)) other.name = (*scene_obj)["name"].s_val;
	if (scene_obj->has("assets", JOBJECT))
	{
		FJsonObject* asset_block = (*scene_obj)["assets"].o_val;
		if (asset_block->has("materials", JARRAY))
		{
			vector<FJsonElement> material_array = (*asset_block)["materials"].a_val;
			for (FJsonElement material : material_array)
			{
				if (material.type == JOBJECT && material.o_val != nullptr)
				{
					if (material.o_val->has("path", JSTRING))
					{
						string path = (*material.o_val)["path"].s_val;
						FJsonBlob material_blob(path);
						FJsonElement mat_root = material_blob.getRoot();
						if (mat_root.type == JOBJECT && mat_root.o_val != nullptr)
						{
							FMaterialPreload mp;
							mat_root >> mp;
							rm->createMaterial(path, mp);
						}
					}
				}
			}
		}
	}
	if (scene_obj->has("objects", JARRAY))
	{
		for (FJsonElement obj : (*scene_obj)["objects"].a_val)
		{
			FObjectPreload o;
			obj >> o;
			other.queueForPreload(o);
		}
	}
	if (scene_obj->has("ambient_light", JARRAY))
		(*scene_obj)["ambient_light"] >> other.ambient_light;
	if (scene_obj->has("fog_colour", JARRAY))
		(*scene_obj)["fog_colour"] >> other.fog_colour;
	if (scene_obj->has("fog_start", JFLOAT))
		other.fog_start = (*scene_obj)["fog_start"].f_val;
	if (scene_obj->has("fog_end", JFLOAT))
		other.fog_end = (*scene_obj)["fog_end"].f_val;
	if (scene_obj->has("fog_strength", JFLOAT))
		other.fog_strength = (*scene_obj)["fog_strength"].f_val;

	return true;
}