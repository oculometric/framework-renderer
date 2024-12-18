#include "FResourceManager.h"

#include <Windows.h>

#include "FMesh.h"
#include "FGraphicsEngine.h"
#include "FDebug.h"
#include "FJsonParser.h"

using namespace std;

static FResourceManager* resource_manager;

void FResourceManager::set(FResourceManager* manager)
{
	resource_manager = manager;
}

FResourceManager* FResourceManager::get()
{
	return resource_manager;
}

FTexture* FResourceManager::loadTexture(string path)
{
	FResource descriptor{ path, FResourceType::TEXTURE };
	if (registry.count(descriptor) > 0)
		return (FTexture*)(registry[descriptor]);

	int wlen = MultiByteToWideChar(CP_UTF8, 0, &path[0], (int)path.size(), nullptr, 0);
	wstring wstr;
	wstr.resize(wlen);
	MultiByteToWideChar(CP_UTF8, 0, &path[0], (int)path.size(), &wstr[0], wlen);
	for (int i = 0; i < wstr.size(); i++) if (wstr[i] == '/') wstr[i] = '\\';
	FTexture* res = application->registerTexture(wstr);
	if (res == nullptr)
	{
		FDebug::dialog("unable to load texture: " + path);
	}
	registry.insert_or_assign(descriptor, (void*)res);

	return res;
}

bool FResourceManager::unloadTexture(FTexture* res)
{
	if (res == nullptr) return false;

	application->unregisterTexture(res);

	return unload(res);
}

FMeshData* FResourceManager::loadMesh(string path)
{
	FResource descriptor{ path, FResourceType::MESH_DATA };
	if (registry.count(descriptor) > 0)
		return (FMeshData*)(registry[descriptor]);

	string fixed_path = path;
	for (int i = 0; i < fixed_path.size(); i++) if (fixed_path[i] == '/') fixed_path[i] = '\\';
	FMeshData* res = FMesh::loadMesh(path);
	if (application->registerMesh(res))
		registry.insert_or_assign(descriptor, (void*)res);
	else
	{
		FDebug::dialog("unable to load mesh: " + path);
		delete res;
		return nullptr;
	}

	return res;
}

bool FResourceManager::unloadMesh(FMeshData* res)
{
	if (res == nullptr) return false;

	application->unregisterMesh(res);
	
	return unload(res);
}

FShader* FResourceManager::loadShader(string path, bool wireframe, FCullMode culling)
{
	FResource descriptor{ path, FResourceType::SHADER };
	if (registry.count(descriptor) > 0)
		return (FShader*)(registry[descriptor]);

	int wlen = MultiByteToWideChar(CP_UTF8, 0, &path[0], (int)path.size(), nullptr, 0);
	wstring wstr;
	wstr.resize(wlen);
	MultiByteToWideChar(CP_UTF8, 0, &path[0], (int)path.size(), &wstr[0], wlen);
	for (int i = 0; i < wstr.size(); i++) if (wstr[i] == '/') wstr[i] = '\\';
	FShader* res = new FShader();
	res->draw_wireframe = wireframe;
	res->cull_mode = culling;
	if (application->registerShader(res, wstr))
		registry.insert_or_assign(descriptor, (void*)res);
	else
	{
		FDebug::dialog("unable to load shader: " + path);
		delete res;
		return nullptr;
	}

	return res;
}

bool FResourceManager::unloadShader(FShader* res)
{
	if (res == nullptr) return false;

	application->unregisterShader(res);

	return unload(res);
}

FMaterial* FResourceManager::loadMaterial(string name)
{
	FResource descriptor{ name, FResourceType::MATERIAL };
	if (registry.count(descriptor) > 0)
		return (FMaterial*)(registry[descriptor]);

	FJsonBlob material_blob(name);
	FJsonElement a = material_blob.getRoot();
	if (a.type != JOBJECT) return nullptr;
	if (a.o_val == nullptr) return nullptr;

	vector<string> textures;
	string shader_name;
	map<string, FMaterialParameter> parameters;
	bool wireframe = false;
	FCullMode culling = FCullMode::BACK;
	FJsonObject* obj = a.o_val;

	if (obj->has("shader", JSTRING)) shader_name = (*obj)["shader"].s_val;
	else return nullptr;
	if (obj->has("textures", JARRAY))
	{
		vector<FJsonElement> texs = (*obj)["textures"].a_val;
		for (FJsonElement t : texs)
			if (t.type == JSTRING) textures.push_back(t.s_val);
	}
	if (obj->has("uniforms", JARRAY))
	{
		for (FJsonElement el : (*obj)["uniforms"].a_val)
		{
			if (el.type != JOBJECT || el.o_val == nullptr) continue;
			FJsonObject* eobj = el.o_val;
			if (!eobj->has("type", JSTRING) || !eobj->has("param", JSTRING)) continue;
			
			FMaterialParameter param;

			string type = (*eobj)["type"].s_val;
			string name = (*eobj)["param"].s_val;
			if (eobj->elements.count("value") < 1) continue;
			FJsonElement value = (*eobj)["value"];

			if (type == "int")
			{
				if (value.type != JFLOAT) continue;
				param = FMaterialParameter((INT)value.f_val);
			}
			else if (type == "float")
			{
				if (value.type != JFLOAT) continue;
				param = FMaterialParameter((FLOAT)value.f_val);
			}
			else if (type == "int3")
			{
				if (value.type != JARRAY) continue;
				XMINT3 i; value >> i;
				param = FMaterialParameter(i);
			}
			else if (type == "float3")
			{
				if (value.type != JARRAY) continue;
				XMFLOAT3 f; value >> f;
				param = FMaterialParameter(f);
			}
			else if (type == "float4")
			{
				if (value.type != JARRAY) continue;
				XMFLOAT4 f; value >> f;
				param = FMaterialParameter(f);
			}

			parameters.insert_or_assign(name, param);
		}
	}
	if (obj->has("wireframe", JFLOAT))
		wireframe = (*obj)["wireframe"].f_val > 0.0f;
	if (obj->has("culling", JSTRING))
	{
		string c = (*obj)["culling"].s_val;
		if (c == "FRONT")
			culling = FCullMode::FRONT;
		else if (c == "OFF")
			culling = FCullMode::OFF;
		else
			culling = FCullMode::BACK;
	}

	FShader* shader = nullptr;
	shader = loadShader(shader_name, wireframe, culling);
	if (shader == nullptr)
	{
		FDebug::dialog("unable to load material: " + name);
		return nullptr;
	}

	FMaterial* res = new FMaterial();
	res->shader = shader;
	res->parameters = parameters;
	size_t i = 0;
	for (string tex_name : textures)
	{
		if (tex_name.empty()) { i++; continue; }

		res->assignTexture(loadTexture(tex_name), i);
		i++;
	}

	registry.insert_or_assign(descriptor, (void*)res);

	return res;
}

bool FResourceManager::unloadMaterial(FMaterial* mat)
{
	if (mat == nullptr) return false;

	return unload(mat);
}

FResourceManager::~FResourceManager()
{
	vector<pair<FResource, void*>> pairs;
	for (pair<FResource, void*> res_pair : registry)
		pairs.push_back(res_pair);

	for (pair<FResource, void*> res_pair : pairs)
	{
		switch (res_pair.first.type)
		{
		case FResourceType::TEXTURE:   unloadTexture((FTexture*)(res_pair.second)); break;
		case FResourceType::MESH_DATA: unloadMesh((FMeshData*)(res_pair.second)); break;
		case FResourceType::SHADER:    unloadShader((FShader*)(res_pair.second)); break;
		case FResourceType::MATERIAL:  unloadMaterial((FMaterial*)(res_pair.second)); break;
		}
	}
}

bool FResourceManager::unload(void* res)
{
	for (pair<FResource, void*> res_pair : registry)
	{
		if (res_pair.second == res)
		{
			registry.erase(res_pair.first);
			delete res;
			return true;
		}
	}
	return false;
}
