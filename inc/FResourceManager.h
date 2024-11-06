#pragma once

#include <string>
#include <map>
#include <vector>

#include "FMaterial.h"

class FApplication;
class FGraphicsEngine;
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
		std::string name;
		FResourceType type;

		inline bool operator<(const FResource& b) const { return (name < b.name); }
	};

	FGraphicsEngine* application = nullptr;

	std::map<FResource, void*> registry;

	inline FResourceManager(FGraphicsEngine* engine) : application(engine) { }

	static void set(FResourceManager* manager);

public:
	static FResourceManager* get();

	FTexture*  loadTexture(std::string path);
	bool       unloadTexture(FTexture* res);

	FMeshData* loadMesh(std::string path);
	bool       unloadMesh(FMeshData* res);

	FShader*   loadShader(std::string path, bool wireframe, FCullMode culling);
	bool       unloadShader(FShader* res);

	FMaterial* createMaterial(std::string name, FMaterialPreload mp);
	FMaterial* getMaterial(std::string name);
	bool	   unloadMaterial(FMaterial* mat);

	~FResourceManager();

private:
	bool	   unload(void* res);
};