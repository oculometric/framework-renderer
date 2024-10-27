#pragma once

#include <string>
#include <map>
#include <vector>

#include "FMaterial.h"

using namespace std;

class FApplication;
class FTexture;
class FMeshData;
class FShader;
class FResourceManager;

class FResourceManager
{
	friend class FApplication;
private:
	enum FResourceType
	{
		TEXTURE,
		MESH_DATA,
		SHADER,
		MATERIAL,
		CAMERA_DATA
	};

	struct FResource
	{
		string name;
		FResourceType type;

		inline bool operator<(const FResource& b) const
		{
			return (name < b.name);
		}
	};

	FApplication* application;

	map<FResource, void*> registry;

	inline FResourceManager(FApplication* app) : application(app) { }

	static void set(FResourceManager* manager);

public:
	static FResourceManager* get();

	FTexture*  loadTexture(string path);
	bool       unloadTexture(FTexture* res);

	FMeshData* loadMesh(string path);
	bool       unloadMesh(FMeshData* res);

	FShader*   loadShader(string path, bool wireframe, FCullMode culling);
	bool       unloadShader(FShader* res);

	FMaterial* createMaterial(string name, FMaterialPreload mp);
	FMaterial* getMaterial(string name);
	bool	   unloadMaterial(FMaterial* mat);

	~FResourceManager();

private:
	bool	   unload(void* res);
};