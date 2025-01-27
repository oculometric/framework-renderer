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

inline FVector closestPointOnBox(const FBoundingBox& on, const FVector& to)
{
	FVector closest_point = to;
	if (closest_point.x > on.max_corner.x) closest_point.x = on.max_corner.x;
	else if (closest_point.x < on.min_corner.x) closest_point.x = on.min_corner.x;
	if (closest_point.y > on.max_corner.y) closest_point.y = on.max_corner.y;
	else if (closest_point.y < on.min_corner.y) closest_point.y = on.min_corner.y;
	if (closest_point.z > on.max_corner.z) closest_point.z = on.max_corner.z;
	else if (closest_point.z < on.min_corner.z) closest_point.z = on.min_corner.z;

	return closest_point;
}

inline FVector getCenter(const FBoundingBox& box)
{
	return (box.min_corner + box.max_corner) / 2.0f;
}

inline FVector getExtents(const FBoundingBox& box)
{
	return box.max_corner - box.min_corner;
}