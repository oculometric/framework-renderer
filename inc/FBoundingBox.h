#pragma once

#include "FVector.h"

// structure for storing a bounding box
struct FBoundingBox
{
	FVector min_corner = FVector(0, 0, 0);
	FVector max_corner = FVector(0, 0, 0);

	inline FBoundingBox(FVector origin, FVector extents)
	{
		min_corner = origin - (extents / 2.0f);
		max_corner = origin + (extents / 2.0f);
	}

	inline FBoundingBox() { }
};