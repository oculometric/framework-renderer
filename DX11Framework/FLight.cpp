#include "FLight.h"

void FLight::convertToData(FLightData* data)
{
	data->colour = colour;
	data->strength = strength;

	data->angle = angle;

	// calculate direction from world-matrix
	XMFLOAT4X4 world_mat = getTransform();
	XMFLOAT3 direction = XMFLOAT3(-world_mat._31, -world_mat._32, -world_mat._33);

	data->light_direction = XMFLOAT4(direction.x, direction.y, direction.z, (float)(type == FLightType::DIRECTIONAL ? 0 : 1));
	data->light_position = XMFLOAT3(world_mat._41, world_mat._42, world_mat._43);
}
