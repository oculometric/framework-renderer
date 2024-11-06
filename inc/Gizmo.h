#pragma once

#include <vector>
#include <DirectXMath.h>

#include "FMesh.h"

using namespace DirectX;

std::vector<FVertex> gizmo_verts =
{
	FVertex{ XMFLOAT3(0, 0, 0),			XMFLOAT4(1, 1, 1, 1) },

	FVertex{ XMFLOAT3(1, 0, 0),			XMFLOAT4(1, 0, 0, 1) },
	FVertex{ XMFLOAT3(0.8f, 0.2f, 0),	XMFLOAT4(1, 0, 0, 1) },
	FVertex{ XMFLOAT3(0.8f, -0.2f, 0),	XMFLOAT4(1, 0, 0, 1) },
	FVertex{ XMFLOAT3(0.8f, 0, 0.2f),	XMFLOAT4(1, 0, 0, 1) },
	FVertex{ XMFLOAT3(0.8f, 0, -0.2f),	XMFLOAT4(1, 0, 0, 1) },

	FVertex{ XMFLOAT3(0, 1, 0),			XMFLOAT4(0, 1, 0, 1) },
	FVertex{ XMFLOAT3(0.2f, 0.8f, 0),	XMFLOAT4(0, 1, 0, 1) },
	FVertex{ XMFLOAT3(-0.2f, 0.8f, 0),	XMFLOAT4(0, 1, 0, 1) },
	FVertex{ XMFLOAT3(0, 0.8f, 0.2f),	XMFLOAT4(0, 1, 0, 1) },
	FVertex{ XMFLOAT3(0, 0.8f, -0.2f),	XMFLOAT4(0, 1, 0, 1) },

	FVertex{ XMFLOAT3(0, 0, 1),			XMFLOAT4(0, 0, 1, 1) },
	FVertex{ XMFLOAT3(0.2f, 0, 0.8f),	XMFLOAT4(0, 0, 1, 1) },
	FVertex{ XMFLOAT3(-0.2f, 0, 0.8f),	XMFLOAT4(0, 0, 1, 1) },
	FVertex{ XMFLOAT3(0, 0.2f, 0.8f),	XMFLOAT4(0, 0, 1, 1) },
	FVertex{ XMFLOAT3(0, -0.2f, 0.8f),	XMFLOAT4(0, 0, 1, 1) }
};

std::vector<uint16_t> gizmo_inds =
{
	0, 1,
	1, 2,
	1, 3,
	1, 4,
	1, 5,

	0, 6,
	6, 7,
	6, 8,
	6, 9,
	6, 10,
	
	0, 11,
	11, 12,
	11, 13,
	11, 14,
	11, 15
};

std::vector<FVertex> box_verts =
{
	FVertex{ XMFLOAT3(-1,-1,-1),	XMFLOAT4(0,0,0,1) },
	FVertex{ XMFLOAT3(1,-1,-1),		XMFLOAT4(1,0,0,1) },
	FVertex{ XMFLOAT3(1,1,-1),		XMFLOAT4(1,1,0,1) },
	FVertex{ XMFLOAT3(-1,1,-1),		XMFLOAT4(0,1,0,1) },
	FVertex{ XMFLOAT3(-1,-1,1),		XMFLOAT4(0,0,1,1) },
	FVertex{ XMFLOAT3(1,-1,1),		XMFLOAT4(1,0,1,1) },
	FVertex{ XMFLOAT3(1,1,1),		XMFLOAT4(1,1,1,1) },
	FVertex{ XMFLOAT3(-1,1,1),		XMFLOAT4(0,1,1,1) },
};

std::vector<uint16_t> box_inds =
{
	0, 1,
	1, 2,
	2, 3,
	3, 0,
	4, 5,
	5, 6,
	6, 7,
	7, 4,
	0, 4,
	1, 5,
	2, 6,
	3, 7
};