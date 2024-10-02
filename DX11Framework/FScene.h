class FScene;

#ifndef FSCENE_H
#define FSCENE_H

#include <DirectXMath.h>

#include "FObject.h"

class FApplication;

using namespace DirectX;

class FScene
{
	friend class FApplication;

private:
	FObject root;
	FApplication* owner = nullptr;

protected:
	vector<FObject*> all_objects;

public:
	FScene() = delete;
	explicit inline FScene(FApplication* application) { owner = application; }
	FScene(FScene& other) = delete;
	FScene(FScene&& other) = delete;

	void addObject(FObject* o, FObject* parent);

	inline virtual void start() { }
	inline virtual void update(float delta_time) { }
};

#endif