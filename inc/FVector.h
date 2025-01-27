#pragma once

#include <string>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <DirectXMath.h>

typedef DirectX::XMFLOAT3 FVector;
#else
struct FVector
{
	float x, y, z;

	constexpr FVector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }
};
#endif

// vectorwise operations

inline FVector operator+(const FVector& a, const FVector& b) { return FVector(a.x + b.x, a.y + b.y, a.z + b.z); }
inline FVector operator-(const FVector& a, const FVector& b) { return FVector(a.x - b.x, a.y - b.y, a.z - b.z); }
inline FVector operator*(const FVector& a, const FVector& b) { return FVector(a.x * b.x, a.y * b.y, a.z * b.z); }
inline FVector operator/(const FVector& a, const FVector& b) { return FVector(a.x / b.x, a.y / b.y, a.z / b.z); }

inline FVector operator-(const FVector& a) { return FVector(-a.x, -a.y, -a.z); }

// bulk operations

inline FVector operator+(const FVector& a, float f) { return FVector(a.x + f, a.y + f, a.z + f); }
inline FVector operator-(const FVector& a, float f) { return FVector(a.x - f, a.y - f, a.z - f); }
inline FVector operator*(const FVector& a, float f) { return FVector(a.x * f, a.y * f, a.z * f); }
inline FVector operator/(const FVector& a, float f) { return FVector(a.x / f, a.y / f, a.z / f); }

// dot product
inline float operator^(const FVector& a, const FVector& b) { return (a.x * b.x) + (a.y * b.y) + (a.z * b.z); }

// cross product
inline FVector operator%(const FVector& a, const FVector& b) { return FVector((a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x)); }

inline float magnitude_squared(const FVector& a) { return a ^ a; }

inline float magnitude(const FVector& a) { return sqrt(magnitude_squared(a)); }

inline FVector normalise(const FVector& a) { return a / magnitude(a); }

inline FVector abs(const FVector& a) { return FVector(abs(a.x), abs(a.y), abs(a.z)); }

inline float angle_between(const FVector& a, const FVector& b) { return acos((a ^ b) / (magnitude(a) * magnitude(b))); }

// reflects vector a in the plane defined by the normal vector b (assumes that b is normalised)
inline FVector reflect(const FVector& a, const FVector& b) { return a - (b * (a ^ b) * 2.0f); }

// finds the closest point on a line defined by r = a + Ld to a point p (assumes that d is normalised)
inline FVector project(const FVector& d, const FVector& a, const FVector& p) { return a + (d * ((p - a) ^ d)); }

// finds the intersection point between a line (r = la + Ld) and a plane ((r - pa) ^ n = 0) (assumes that d and n are normalised)
inline FVector intersect(const FVector& n, const FVector& pa, const FVector& d, const FVector& la) { la + (d * ((pa - la) ^ n) / (d ^ n)); }

// rotates vector v about axis a, relative to origin o, by angle t radians (assumes that a is normalised)
inline FVector rotate(const FVector& v, const FVector& o, const FVector& a, float t)
{
	FVector v_par = a * (((v - o) ^ a) / (a ^ a));
	FVector v_per = (v - o) - v_par;
	FVector w = a % v_per;
	return (v_per * cos(t)) + ((w * sin(t) * magnitude(v_per)) / magnitude(w)) + v_par + o;
}

inline std::string str(const FVector& v)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(3) << "( " << v.x << ", " << v.y << ", " << v.z << " )";
	return stream.str();
}
