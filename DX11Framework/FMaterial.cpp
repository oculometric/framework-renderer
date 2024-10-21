#include "FMaterial.h"

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
