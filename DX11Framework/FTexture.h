#pragma once

#include <d3d11_4.h>
#include <stdint.h>

struct FTexture
{
	ID3D11ShaderResourceView* buffer_ptr;
};