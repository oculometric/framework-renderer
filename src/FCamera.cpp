#include "FObject.h"

void FCameraComponent::updateProjectionMatrix()
{
	XMStoreFloat4x4(&projection_matrix, XMMatrixPerspectiveFovRH(XMConvertToRadians(field_of_view), aspect_ratio, near_clip, far_clip));
}
