#include "FMaterial.h"

#include "FJsonParser.h"

using namespace std;

void FMaterial::setParameter(string name, FMaterialParameter param)
{
	parameters.insert_or_assign(name, param);
}

FMaterialParameter FMaterial::getParameter(string name)
{
	if (parameters.count(name) > 0) return parameters[name];
	return FMaterialParameter();
}

void FMaterial::assignTexture(FTexture* tex, size_t index)
{
	if (index < MAX_TEXTURES) textures[index] = tex;
}

bool operator>>(const FJsonElement& a, FMaterialPreload& other)
{
	if (a.type != JOBJECT) return false;
	if (a.o_val == nullptr) return false;

	FJsonObject* obj = a.o_val;

	if (obj->has("shader", JSTRING)) other.shader = (*obj)["shader"].s_val;
	if (obj->has("textures", JARRAY))
	{
		vector<FJsonElement> texs = (*obj)["textures"].a_val;
		for (FJsonElement t : texs)
			if (t.type == JSTRING) other.textures.push_back(t.s_val);
	}
	if (obj->has("uniforms", JARRAY))
	{
		for (FJsonElement el : (*obj)["uniforms"].a_val)
		{
			if (el.type != JOBJECT || el.o_val == nullptr) continue;
			FJsonObject* eobj = el.o_val;
			if (!eobj->has("type", JSTRING) || !eobj->has("param", JSTRING)) continue;
			
			FMaterialParameter param;

			string type = (*eobj)["type"].s_val;
			string name = (*eobj)["param"].s_val;
			if (eobj->elements.count("value") < 1) continue;
			FJsonElement value = (*eobj)["value"];

			if (type == "int")
			{
				if (value.type != JFLOAT) continue;
				param = FMaterialParameter((INT)value.f_val);
			}
			else if (type == "float")
			{
				if (value.type != JFLOAT) continue;
				param = FMaterialParameter((FLOAT)value.f_val);
			}
			else if (type == "int3")
			{
				if (value.type != JARRAY) continue;
				XMINT3 i; value >> i;
				param = FMaterialParameter(i);
			}
			else if (type == "float3")
			{
				if (value.type != JARRAY) continue;
				XMFLOAT3 f; value >> f;
				param = FMaterialParameter(f);
			}
			else if (type == "float4")
			{
				if (value.type != JARRAY) continue;
				XMFLOAT4 f; value >> f;
				param = FMaterialParameter(f);
			}

			other.parameters.insert_or_assign(name, param);
		}
	}
	if (obj->has("wireframe", JFLOAT))
		other.wireframe = (*obj)["wireframe"].f_val > 0.0f;
	if (obj->has("culling", JSTRING))
	{
		string c = (*obj)["culling"].s_val;
		if (c == "FRONT")
			other.culling = FCullMode::FRONT;
		else if (c == "OFF")
			other.culling = FCullMode::OFF;
		else
			other.culling = FCullMode::BACK;
	}
	else
		other.culling = FCullMode::BACK;

	return true;
}