#pragma once

#include "FVector.h"

// structure for storing a bounding box
struct FBoundingBox
{
	FVector max_corner = FVector(0, 0, 0);
	FVector min_corner = FVector(0, 0, 0);
};