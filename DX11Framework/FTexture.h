#pragma once

#include <d3d11_4.h>
#include <stdint.h>

class FTexture
{
	friend class FGraphicsEngine;
private:
	inline FTexture() { };

	ID3D11ShaderResourceView* buffer_ptr = nullptr;
};