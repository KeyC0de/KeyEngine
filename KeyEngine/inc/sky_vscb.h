#pragma once

#include <DirectXMath.h>
#include "constant_buffer.h"


class Camera;

class SkyVSCB
	: public IBindable
{
	struct Transform
	{
		DirectX::XMMATRIX viewProjection;
	};
	std::unique_ptr<VertexShaderConstantBuffer<Transform>> m_pVscb;
public:
	SkyVSCB( Graphics &gfx, const unsigned slot = 0u );

	void bind( Graphics &gfx ) cond_noex override;
	Transform getTransform( Graphics &gfx ) cond_noex;
};