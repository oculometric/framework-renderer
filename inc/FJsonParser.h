#pragma once

#include <map>
#include <string>
#include <vector>
#include <DirectXMath.h>

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
	
	std::string s_val;
	std::vector<FJsonElement> a_val;
	union
	{
		float f_val;
		FJsonObject* o_val = nullptr;
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
	inline FJsonElement(std::string s) { s_val = s; type = FJsonType::JSTRING; }
	inline FJsonElement(FJsonObject* o) { o_val = o; type = FJsonType::JOBJECT; }
	inline FJsonElement(std::vector<FJsonElement> a) { a_val = a; type = FJsonType::JARRAY; }
	inline FJsonElement(const FJsonElement& other) { assign(other); }
	inline FJsonElement(const FJsonElement&& other) noexcept { assign(other); }
	inline FJsonElement operator=(const FJsonElement& other) { assign(other); return *this; }
	inline FJsonElement operator=(const FJsonElement&& other) noexcept { assign(other); return *this; }
	inline ~FJsonElement() { if (type == FJsonType::JARRAY) a_val.~vector(); }
};

struct FJsonObject
{
	std::map<std::string, FJsonElement> elements = std::map<std::string, FJsonElement>({ });

	inline FJsonObject() { }
	
	inline bool has(std::string s, FJsonType t) { if (elements.count(s) <= 0) return false; return elements.at(s).type == t; }
	inline FJsonElement operator[](std::string s) { return elements.at(s); }
	inline FJsonElement operator[](const char* s) { return elements.at(s); }
};

class FJsonBlob
{
private:
	std::vector<FJsonObject*> all_objects;
	FJsonElement root = FJsonElement(nullptr);

	bool validate(const std::string& s);
	size_t next(const std::string& s, const size_t start, const char delim);
	std::string extract(const std::string& s, const size_t start, size_t& end);
	FJsonElement decode(const std::string& s);
	FJsonObject* parse(const std::string& s);
	std::string reduce(const std::string& s);

public:
	FJsonBlob(std::string path);
	FJsonBlob() = delete;
	FJsonBlob(const FJsonBlob& other) = delete;
	FJsonBlob(const FJsonBlob&& other) = delete;
	FJsonBlob operator=(const FJsonBlob& other) = delete;
	FJsonBlob operator=(const FJsonBlob&& other) = delete;

	inline FJsonElement getRoot() const { return root; }

	~FJsonBlob();
};

// override this in order to parse specific classes out of FJsonObjects
// inline bool operator>>(const FJsonElement& a, T& other) { };

class FScene;
struct FObjectPreload;
struct FMaterialPreload;

bool operator>>(const FJsonElement& a, FScene& other);
bool operator>>(const FJsonElement& a, FObjectPreload& other);
bool operator>>(const FJsonElement& a, FMaterialPreload& other);
bool operator>>(const FJsonElement& a, XMFLOAT3& other);
bool operator>>(const FJsonElement& a, XMFLOAT4& other);
bool operator>>(const FJsonElement& a, XMINT3& other);