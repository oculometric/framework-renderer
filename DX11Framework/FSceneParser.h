#pragma once

#include <map>
#include <string>
#include <vector>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

enum FJsonType
{
	JFLOAT,
	JSTRING,
	JOBJECT,
	JARRAY
};

struct FJsonObject;

struct FJsonElement
{
	FJsonType type;
	
	string s_val;
	vector<FJsonElement> a_val;
	union
	{
		float f_val;
		FJsonObject* o_val;
	};

private:
	inline void assign(const FJsonElement& other)
	{
		type = other.type;
		a_val = other.a_val;
		s_val = other.s_val;
		o_val = other.o_val;
	}

public:

	inline FJsonElement(float f) { f_val = f; type = FJsonType::JFLOAT; }
	inline FJsonElement(string s) { s_val = s; type = FJsonType::JSTRING; }
	inline FJsonElement(FJsonObject* o) { o_val = o; type = FJsonType::JOBJECT; }
	inline FJsonElement(vector<FJsonElement> a) { a_val = a; type = FJsonType::JARRAY; }
	inline FJsonElement(const FJsonElement& other) { assign(other); }
	inline FJsonElement(const FJsonElement&& other) { assign(other); }
	inline FJsonElement operator=(const FJsonElement& other) { assign(other); return *this; }
	inline FJsonElement operator=(const FJsonElement&& other) { assign(other); return *this; }
	inline ~FJsonElement() { if (FJsonType::JARRAY) a_val.~vector(); }
};

struct FJsonObject
{
	map<string, FJsonElement> elements = map<string, FJsonElement>({ });

	inline FJsonObject() { }
	
};

class FJsonBlob
{
private:
	vector<FJsonObject*> all_objects;
	FJsonObject* root = nullptr;

	bool validate(const string& s);
	size_t next(const string& s, const size_t start, const char delim);
	string extract(const string& s, const size_t start, size_t& end);
	FJsonElement decode(const string& s);
	FJsonObject* parse(const string& s);
	string reduce(const string& s);

public:
	FJsonBlob(string path);
	FJsonBlob() = delete;
	FJsonBlob(const FJsonBlob& other) = delete;
	FJsonBlob(const FJsonBlob&& other) = delete;
	FJsonBlob operator=(const FJsonBlob& other) = delete;
	FJsonBlob operator=(const FJsonBlob&& other) = delete;

	inline FJsonObject* getRoot() const { return root; }

	~FJsonBlob();
};

// override this in order to parse specific classes out of FJsonObjects
template <typename T>
bool operator>>(const FJsonElement& a, T& other);

class FScene;
class FObject;
class FMesh;
class FCamera;
class FMaterial;
class FShader;

bool operator>>(const FJsonElement& a, FScene& other);
bool operator>>(const FJsonElement& a, FObject& other);
bool operator>>(const FJsonElement& a, FMesh& other);
bool operator>>(const FJsonElement& a, FCamera& other);
bool operator>>(const FJsonElement& a, FMaterial& other);
bool operator>>(const FJsonElement& a, FShader& other);
