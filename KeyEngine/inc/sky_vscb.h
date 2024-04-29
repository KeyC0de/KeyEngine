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
	SkyVSCB( Graphics &gph, const unsigned slot = 0u );

	void bind( Graphics &gph ) cond_noex override;
	Transform getTransform( Graphics &gph ) cond_noex;
};