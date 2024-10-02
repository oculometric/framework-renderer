#include "FCamera.h"

void FCamera::lookAt(XMFLOAT3 eye, XMFLOAT3 target, XMFLOAT3 up)
{
	XMFLOAT4X4 look;
	XMStoreFloat4x4(&look, XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up)));
	// TODO: lookat
}

void FCamera::updateProjectionMatrix()
{
	XMStoreFloat4x4(&projection_matrix, XMMatrixPerspectiveFovLH(XMConvertToRadians(configuration.field_of_view), configuration.aspect_ratio, configuration.near_clip, configuration.far_clip));
}
