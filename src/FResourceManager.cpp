#include "FResourceManager.h"

#include <Windows.h>

#include "FMesh.h"
#include "FGraphicsEngine.h"
#include "FDebug.h"

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

FMaterial* FResourceManager::createMaterial(string name, FMaterialPreload mp)
{
	FResource descriptor{ name, FResourceType::MATERIAL };
	if (registry.count(descriptor) > 0)
		return (FMaterial*)(registry[descriptor]);

	FShader* shader = nullptr;
	shader = loadShader(mp.shader, mp.wireframe, mp.culling);
	if (shader == nullptr)
	{
		FDebug::dialog("unable to load material: " + name);
		return nullptr;
	}

	FMaterial* res = new FMaterial();
	res->shader = shader;
	res->parameters = mp.parameters;
	size_t i = 0;
	for (string tex_name : mp.textures)
	{
		if (tex_name.empty()) { i++; continue; }

		res->assignTexture(loadTexture(tex_name), i);
		i++;
	}

	registry.insert_or_assign(descriptor, (void*)res);

	return res;
}

FMaterial* FResourceManager::getMaterial(string name)
{
	FResource descriptor{ name, FResourceType::MATERIAL };
	if (registry.count(descriptor) > 0)
		return (FMaterial*)(registry[descriptor]);

	return nullptr;
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
