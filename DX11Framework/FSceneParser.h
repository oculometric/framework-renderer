#pragma once

#include <map>
#include <string>
#include <vector>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

enum FJsonType
{
	FLOAT,
	VECTOR,
	INT,
	STRING,
	OBJECT,
	ARRAY
};

struct FJsonObject;

struct FJsonElement
{
	FJsonType type;
	
	union
	{
		float f_val;
		XMFLOAT3 v_val;
		int i_val;
		string s_val;
		FJsonObject* o_val;
		vector<FJsonElement> a_val;
	};
};

struct FJsonObject
{
	map<string, FJsonElement> elements;
};

class FJsonBlob
{
private:
	vector<FJsonObject*> all_objects;
	FJsonObject* root = nullptr;

	bool validate(string s);
	string extractBlock(string s, size_t start, char delim);
	FJsonObject* parseBlock(string s);
	string reduce(string s);

public:
	FJsonBlob(string path);
	FJsonBlob() = delete;
	FJsonBlob(const FJsonBlob& other) = delete;
	FJsonBlob(const FJsonBlob&& other) = delete;
	FJsonBlob operator=(const FJsonBlob& other) = delete;
	FJsonBlob operator=(const FJsonBlob&& other) = delete;

	inline FJsonObject* getRoot() { return root; }
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
