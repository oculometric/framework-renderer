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
		// TODO: convert loading these things into >>'ing them!
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
			vector<FJsonElement> mesh_array = (*asset_block)["textures"].a_val;
			for (FJsonElement mesh : mesh_array)
			{
				if (mesh.type == JOBJECT && mesh.o_val != nullptr)
					if (mesh.o_val->has("path", JSTRING)) rm->loadTexture((*mesh.o_val)["path"].s_val);
			}
		}
		// TODO: load materials
		// TODO: load shaders
	}
	// TODO: load object tree
	// TODO: find a camera

	return true;
}