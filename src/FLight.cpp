#include "FLight.h"

void FLight::convertToData(FLightData* data)
{
	data->colour = colour;
	data->strength = strength;

	data->angle = (type == FLightType::SPOT ? angle : 180.0f);

	// calculate direction from world-matrix
	XMFLOAT4X4 world_mat = transform.getTransform();
	XMFLOAT3 direction = XMFLOAT3(-world_mat._31, -world_mat._32, -world_mat._33);

	data->light_direction = XMFLOAT4(direction.x, direction.y, direction.z, (float)(type == FLightType::DIRECTIONAL ? 0 : 1));
	data->light_position = XMFLOAT3(world_mat._41, world_mat._42, world_mat._43);
}

XMFLOAT4X4 FLight::getProjectionMatrix()
{
	// FIXME: this only works for spotlights! uhhhh....
	XMFLOAT4X4 projection_matrix;
	if (type == SPOT)
		XMStoreFloat4x4(&projection_matrix, XMMatrixPerspectiveFovRH(XMConvertToRadians(angle * 2.0f), 1.0f, 0.1f, 16.0f));
	if (type == DIRECTIONAL)
		XMStoreFloat4x4(&projection_matrix, XMMatrixOrthographicRH(8.0f, 8.0f, 0.1f, 16.0f));

	return projection_matrix;
}
