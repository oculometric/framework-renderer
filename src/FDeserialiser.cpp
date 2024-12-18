#include "FDeserialiser.h"

#include "FJsonParser.h"
#include "FComponent.h"
#include "FMesh.h"
#include "FObject.h"
#include "FScene.h"
#include "FResourceManager.h"

using namespace std;

// TODO: put these in the header file
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

bool operator>>(const FJsonElement& a, FComponent*& other, const FObject* object)
{
    if (a.type != JOBJECT) return false;

	FResourceManager* rm = FResourceManager::get();

    FJsonObject* obj = a.o_val;
    if (obj == nullptr) return false;
    if (!obj->has("class", JSTRING)) return false;
    if (other != nullptr) delete other;

	string object_class = (*obj)["class"].s_val;
	if (object_class == "mesh")
	{
        FMesh* other_mesh = new FMesh(object);
		other = other_mesh;
		if (obj->has("data", JSTRING)) other_mesh->loadMesh((*obj)["data"].s_val);
		if (obj->has("material", JSTRING)) other_mesh->setMaterial(rm->getMaterial((*obj)["material"].s_val));
		if (obj->has("cast_shadow", JFLOAT)) other_mesh->cast_shadow = (*obj)["cast_shadow"].f_val > 0.0f;
	}
	else if (object_class == "camera")
	{
        FCameraComponent* other_camera = new FCameraComponent(object);
		other = other_camera;
		if (obj->has("aspect_ratio", JFLOAT)) other_camera->aspect_ratio = (*obj)["aspect_ratio"].f_val;
		if (obj->has("field_of_view", JFLOAT)) other_camera->field_of_view = (*obj)["field_of_view"].f_val;
		if (obj->has("near_clip", JFLOAT)) other_camera->near_clip = (*obj)["near_clip"].f_val;
		if (obj->has("far_clip", JFLOAT)) other_camera->far_clip = (*obj)["far_clip"].f_val;
	}
	else if (object_class == "empty")
		other = new FComponent(object);
	else if (object_class == "light")
	{
        FLight* other_light = new FLight(object);
		other = other_light;
		if (obj->has("colour", JARRAY)) (*obj)["colour"] >> other_light.colour;
		if (obj->has("strength", JFLOAT)) other_light->strength = (*obj)["strength"].f_val;
		if (obj->has("angle", JFLOAT)) other_light->angle = (*obj)["angle"].f_val;
		if (obj->has("type", JSTRING))
        {
            string type = (*obj)["type"].s_val;
            if (type == "directional")
                other_light->type = FLight::FLightType::DIRECTIONAL;
            else if (type == "spot")
                other_light->type = FLight::FLightType::SPOT;
            else if (type == "POINT")
                other_light->type = FLight::FLightType::POINT;
            else return false;
        }
	}
	else
		return false;

    return true;
}

// TODO: remove the preload
bool operator>>(const FJsonElement& a, FObject* other)
{
	if (a.type != JOBJECT) return false;

	FJsonObject* obj = a.o_val;
	if (obj == nullptr) return false;

    XMFLOAT3 tmp;

	if (obj->has("name", JSTRING)) other.name = (*obj)["name"].s_val;
	if (obj->has("position", JARRAY)) { (*obj)["position"] >> tmp; other->transform.setLocalPosition(tmp); }
	if (obj->has("rotation", JARRAY)) { (*obj)["rotation"] >> tmp; other->transform.setLocalEuler(tmp); }
	if (obj->has("scale", JARRAY)) { (*obj)["scale"] >> tmp; other->transform.setLocalScale(tmp); }

    // TODO: load components

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


// TODO: remove the preload
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