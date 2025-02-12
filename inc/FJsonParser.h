#pragma once

#include <map>
#include <string>
#include <vector>
#include <DirectXMath.h>

using namespace DirectX;

// data type for a JSON element
enum FJsonType
{
	JFLOAT,
	JSTRING,
	JOBJECT,
	JARRAY
};

struct FJsonObject;

// represents an element in a JSON file, which is contained within a JSON object, containing some piece of data: either a string, an array of other elements, a float, or an object
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
	// initialisers for various types of JSON element

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

// represents an object in a JSON file, which contains a map linking string keys with element values
struct FJsonObject
{
	std::map<std::string, FJsonElement> elements = std::map<std::string, FJsonElement>({ });

	inline FJsonObject() { }
	
	inline bool has(std::string s, FJsonType t) { if (elements.count(s) <= 0) return false; return elements.at(s).type == t; }
	inline FJsonElement operator[](std::string s) { return elements.at(s); }
	inline FJsonElement operator[](const char* s) { return elements.at(s); }
};

// represents all the data in a JSON file. manages allocated JSON objects and elements, and once parsed, it can be traversed by whatever is being imported via JSON
class FJsonBlob
{
private:
	std::vector<FJsonObject*> all_objects;			// array of allocated JSON objects
	FJsonElement root = FJsonElement(nullptr);		// root element

	bool validate(const std::string& s);			// scans a JSON string to check if it is valid (returning true for validity)
	size_t next(const std::string& s, const size_t start, const char delim);	// finds the next instance of a specific character, after a starting point
	std::string extract(const std::string& s, const size_t start, size_t& end); // extract the contents of a pair of brackets
	FJsonElement decode(const std::string& s);		// decodes a string into a JSON element
	FJsonObject* parse(const std::string& s);		// parses a string into a JSON object (containing a collection of elements)
	std::string reduce(const std::string& s);		// removes comments, newlines, spaces, etc, leaving only the actual content of a JSON file

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

class FScene;

// these operators are are used to translate a JSON element into various data types
bool operator>>(const FJsonElement& a, XMFLOAT3& other);
bool operator>>(const FJsonElement& a, XMFLOAT4& other);
bool operator>>(const FJsonElement& a, XMINT3& other);