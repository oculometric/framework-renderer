#pragma once

#include <d3d11_4.h>
#include <stdint.h>

// simple class representing a texture resource
class FTexture
{
	friend class FGraphicsEngine;
	
private:
	inline FTexture() { };

	// GPU resource for rendering
	ID3D11ShaderResourceView* buffer_ptr = nullptr;
};