#pragma once

#include <DirectXMath.h>

using namespace DirectX;

vector<FVertex> ico_vertices =
{
	FVertex{ XMFLOAT3(0.000000f, -1.000000f, 0.000000f),   },
	FVertex{ XMFLOAT3(0.723600f, -0.447215f, 0.525720f),   },
	FVertex{ XMFLOAT3(-0.276385f, -0.447215f, 0.850640f),  },
	FVertex{ XMFLOAT3(-0.894425f, -0.447215f, 0.000000f),  },
	FVertex{ XMFLOAT3(-0.276385f, -0.447215f, -0.850640f), },
	FVertex{ XMFLOAT3(0.723600f, -0.447215f, -0.525720f),  },
	FVertex{ XMFLOAT3(0.276385f, 0.447215f, 0.850640f),    },
	FVertex{ XMFLOAT3(-0.723600f, 0.447215f, 0.525720f),   },
	FVertex{ XMFLOAT3(-0.723600f, 0.447215f, -0.525720f),  },
	FVertex{ XMFLOAT3(0.276385f, 0.447215f, -0.850640f),   },
	FVertex{ XMFLOAT3(0.894425f, 0.447215f, 0.000000f),    },
	FVertex{ XMFLOAT3(0.000000f, 1.000000f, 0.000000f),    }
};

vector<uint16_t> ico_indices =
{
	0, 1, 2,
	1, 0, 5,
	0, 2, 3,
	0, 3, 4,
	0, 4, 5,
	1, 5, 10,
	2, 1, 6,
	3, 2, 7,
	4, 3, 8,
	5, 4, 9,
	1, 10, 6,
	2, 6, 7,
	3, 7, 8,
	4, 8, 9,
	5, 9, 10,
	6, 10, 11,
	7, 6, 11,
	8, 7, 11,
	9, 8, 11,
	10, 9, 11
};