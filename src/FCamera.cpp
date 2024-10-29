#include "FObject.h"

void FCamera::lookAt(XMFLOAT3 eye, XMFLOAT3 target, XMFLOAT3 up)
{
	XMFLOAT4X4 look;
	XMStoreFloat4x4(&look, XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up)));
	// TODO: lookat, inverse, inverse of world, etc
}

void FCamera::updateProjectionMatrix()
{
	XMStoreFloat4x4(&projection_matrix, XMMatrixPerspectiveFovRH(XMConvertToRadians(field_of_view), aspect_ratio, near_clip, far_clip));
}
