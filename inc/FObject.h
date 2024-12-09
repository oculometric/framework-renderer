#pragma once

#include <DirectXMath.h>
#include <string>
#include <unordered_set>

#include "FTransform.h"
#include "FComponent.h"

using namespace DirectX;

// base object class implementing common properties across all objects
class FObject
{
public:
	std::string name = "Object";				// name for the object, to make debugging easier
	FTransform transform;						// transform component which handles position/rotation/scale of the object, and scene hierarchy (parenthood, childhood)

public:
	inline FObject() { };

	template <class T>
	inline T* getComponent()					// returns a component of the specified type
	{
		static_assert(std::is_base_of<FComponent, T>::value, "T is not a FComponent type");
		T* tmp = new T(nullptr);
		for (FComponent* c : components)
		{
			if (c->getType() == tmp->getType())
				return (T*)c;
		}
		return nullptr;
	}

	inline void addComponent(FComponent* comp)	// adds a new component to the GameObject
	{
		// if the component already has an owner, don't add it
		if (comp->getOwner() != nullptr && comp->getOwner() != this) return;

		components.insert(comp);
		comp->owner = this;
	}

private:
	std::unordered_set<FComponent*> components;	// set containing components which exist on this game object
};