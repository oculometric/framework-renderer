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
	FApplication* owner;

protected:
	vector<FObject*> all_objects;

public:
	FScene(FApplication* application);
	void addObject(FObject* o, FObject* parent);

protected:
	void onBecameActive();
	void update(float delta_time);
};

#endif