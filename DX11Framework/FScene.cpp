#include "FScene.h"

FScene::FScene(FApplication* application)
{
	owner = application;
}

void FScene::addObject(FObject* o, FObject* parent)
{
	if (!o) return;
	if (parent == nullptr)
	{
		all_objects.push_back(o);
		o->getParent() = &root;
		o->updateTransform();
	}
	else
	{
		all_objects.push_back(o);
		o->getParent() = parent;
		o->updateTransform();
		// TODO: check that parent is in the scene (if not, abort)
	}
	// TODO: check that the object is not already in the scene (if not, abort)
}

void FScene::onBecameActive()
{
	// called when the scene becomes the active scene
}

void FScene::update(float delta_time)
{
	// implement your own update method here
}
