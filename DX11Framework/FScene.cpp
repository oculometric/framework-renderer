#include "FScene.h"

void FScene::addObject(FObject* o, FObject* parent)
{
	if (!o) return;
	if (parent == nullptr)
	{
		all_objects.push_back(o);
		root.addChild(o);
		o->updateTransform();
	}
	else
	{
		all_objects.push_back(o);
		parent->addChild(o);
		o->updateTransform();
		// TODO: check that parent is in the scene (if not, abort)
	}
	// TODO: check that the object is not already in the scene (if not, abort)
}