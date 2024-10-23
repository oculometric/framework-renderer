#include "FScene.h"

#include "FJsonParser.h"
#include "FMesh.h"
#include "FCamera.h"
#include "FResourceManager.h"

FScene::FScene(FApplication* application, string scene_file)
{
	owner = application;
	if (!scene_file.empty()) FJsonBlob(scene_file).getRoot() >> *this;
}

void FScene::addObject(FObject* o, FObject* parent)
{
	if (!o) return;
	if (parent == nullptr)
	{
		all_objects.push_back(o);
		root.addChild(o);
		o->updateTransform();
	}
	else
	{
		all_objects.push_back(o);
		parent->addChild(o);
		o->updateTransform();
		// TODO: check that parent is in the scene (if not, abort)
	}
	// TODO: check that the object is not already in the scene (if not, abort)
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

	FObject* obj;
	switch (o.object_type)
	{
	case FObjectType::MESH:
	{
		FMesh* me = new FMesh();
		if (o.data_name != "") me->setData(rm->loadMesh(o.data_name));
		if (o.material_name != "") me->setMaterial(rm->getMaterial(o.material_name));
		obj = me;
		break;
	}
	case FObjectType::CAMERA:
	{
		FCamera* cam = new FCamera();
		cam->updateProjectionMatrix();
		obj = cam;
		active_camera = cam;
		break;
	}
	default:
		obj = new FObject();
		break;
	}
	obj->name = o.name;
	obj->position = o.position;
	obj->eulers = o.rotation;
	obj->scale = o.scale;

	addObject(obj, parent);
	
	for (FObjectPreload c : o.children)
		finalizeObject(c, obj);
	
	if (parent == nullptr)
		obj->updateTransform();
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
		other.object_type = FObjectType::MESH;
		if (obj->has("data", JSTRING)) other.data_name = (*obj)["data"].s_val;
		if (obj->has("material", JSTRING)) other.material_name = (*obj)["material"].s_val;
	}
	else if (object_class == "camera")
	{
		other.object_type = FObjectType::CAMERA;
		// TODO: add support for pointing to a camera resource
	}
	else if (object_class == "empty")
		other.object_type = FObjectType::EMPTY;
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
		if (asset_block->has("meshes", JARRAY))
		{
			vector<FJsonElement> mesh_array = (*asset_block)["meshes"].a_val;
			for (FJsonElement mesh : mesh_array)
			{
				if (mesh.type == JOBJECT && mesh.o_val != nullptr)
					if (mesh.o_val->has("path", JSTRING)) rm->loadMesh((*mesh.o_val)["path"].s_val);
			}
		}
		if (asset_block->has("textures", JARRAY))
		{
			vector<FJsonElement> texture_array = (*asset_block)["textures"].a_val;
			for (FJsonElement texture : texture_array)
			{
				if (texture.type == JOBJECT && texture.o_val != nullptr)
					if (texture.o_val->has("path", JSTRING)) rm->loadTexture((*texture.o_val)["path"].s_val);
			}
		}
		if (asset_block->has("shaders", JARRAY))
		{
			vector<FJsonElement> shader_array = (*asset_block)["shaders"].a_val;
			for (FJsonElement shader : shader_array)
			{
				if (shader.type == JOBJECT && shader.o_val != nullptr)
				{
					FJsonObject* shader_obj = shader.o_val;
					bool wireframe = false; if (shader_obj->has("wireframe", JFLOAT)) wireframe = (*shader_obj)["wireframe"].f_val > 0;
					FCullMode culling = FCullMode::BACK;
					if (shader_obj->has("culling", JSTRING))
					{
						string cull = (*shader_obj)["culling"].s_val;
						if (cull == "off") culling = FCullMode::OFF;
						if (cull == "front") culling = FCullMode::FRONT;
					}
					if (shader_obj->has("path", JSTRING))
						rm->loadShader((*shader_obj)["path"].s_val, wireframe, culling);
				}
			}
		}
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

	return true;
}