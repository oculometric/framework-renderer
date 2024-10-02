class FScene;

#ifndef FSCENE_H
#define FSCENE_H

#include <DirectXMath.h>

#include "FObject.h"
#include "FCamera.h"

class FApplication;

using namespace DirectX;

class FScene
{
	friend class FApplication;

public:
	FCamera* active_camera = nullptr;

private:
	FObject root;
	FApplication* owner = nullptr;

protected:
	vector<FObject*> all_objects;

public:
	FScene() = delete;
	inline FScene(FApplication* application) { owner = application; }
	FScene(FScene& other) = delete;
	FScene(FScene&& other) = delete;

	void addObject(FObject* o, FObject* parent);

	inline virtual void start() { }
	inline virtual void update(float delta_time) { }
};

#endif