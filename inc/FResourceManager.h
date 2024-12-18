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

// manages the resources in use by the active scene(s), and prevents duplication and memory leaks
class FResourceManager
{
	friend class FApplication;

private:
	// type of resource
	enum FResourceType
	{
		TEXTURE,
		MESH_DATA,
		SHADER,
		MATERIAL
	};

	// resource structure, describing the type and path of a particular loaded resource
	struct FResource
	{
		std::string name;
		FResourceType type;

		inline bool operator<(const FResource& b) const { return (name < b.name); }
	};

	// reference to the owning application
	FGraphicsEngine* application = nullptr;

	// registry of loaded assets. the value type is a pointer to any one of the resource types (material, mesh data, texture, shader)
	std::map<FResource, void*> registry;

	inline FResourceManager(FGraphicsEngine* engine) : application(engine) { }

	static void set(FResourceManager* manager);		// sets the static reference in the source file

public:
	static FResourceManager* get();					// gets the static reference from source file

	FTexture*  loadTexture(std::string path);		// load a texture, from a path, or just return a reference to it if already loaded
	bool       unloadTexture(FTexture* res);		// unload a loaded texture

	FMeshData* loadMesh(std::string path);			// load a mesh, from a path, or just return a reference to it if already loaded
	bool       unloadMesh(FMeshData* res);			// unload a loaded mesh

	// load a shader, and configure it with parameters, from a path, or just return a reference to it if already loaded
	FShader*   loadShader(std::string path, bool wireframe, FCullMode culling);
	bool       unloadShader(FShader* res);			// unload a loaded shader

	// load a material according to its definition in a file, or just return a reference to it if already loaded
	FMaterial* loadMaterial(std::string name);
	bool	   unloadMaterial(FMaterial* mat);		// unload a loaded material

	~FResourceManager();

private:
	bool	   unload(void* res);					// unload all loaded resources
};