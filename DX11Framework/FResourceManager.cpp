#include "FResourceManager.h"

#include <Windows.h>

#include "FMesh.h"
#include "FApplication.h"

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
	FResource descriptor{ path, FResourceManager::TEXTURE };
	if (registry.count(descriptor) > 0)
		return (FTexture*)(registry[descriptor]);

	int wlen = MultiByteToWideChar(CP_UTF8, 0, &path[0], (int)path.size(), nullptr, 0);
	wstring wstr;
	wstr.resize(wlen);
	MultiByteToWideChar(CP_UTF8, 0, &path[0], (int)path.size(), &wstr[0], wlen);
	FTexture* res = application->registerTexture(wstr);
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
	FResource descriptor{ path, FResourceManager::MESH_DATA };
	if (registry.count(descriptor) > 0)
		return (FMeshData*)(registry[descriptor]);

	FMeshData* res = FMesh::loadMesh(path);
	application->registerMesh(res);
	registry.insert_or_assign(descriptor, (void*)res);

	return res;
}

bool FResourceManager::unloadMesh(FMeshData* res)
{
	if (res == nullptr) return false;

	application->unregisterMesh(res);
	
	return unload(res);
}

FShader* FResourceManager::loadShader(string path)
{
	// TODO: load a shader

	return nullptr;
}

bool FResourceManager::unloadShader(FShader* res)
{
	if (res == nullptr) return false;

	// TODO: unload a shader

	return unload(res);
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
			return true;
		}
	}
	return false;
}
