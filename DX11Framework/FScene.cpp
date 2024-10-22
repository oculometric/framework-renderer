#include "FScene.h"

#include "FJsonParser.h"
#include "FResourceManager.h"

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
						rm->loadShader((*shader.o_val)["path"].s_val, wireframe, culling);
				}
			}
		}
		// TODO: load materials
	}
	if (scene_obj->has("objects", JARRAY))
	{
		for (FJsonElement obj : (*scene_obj)["objects"].a_val)
		{
			FObject* o = new FObject();
			other.addObject(o, nullptr);
			//obj >> *o; // REENABLE THIS
			o->updateTransform();
		}
	}

	// TODO: walk down object tree, add all to object list, update transforms
	// TODO: find a camera

	return true;
}