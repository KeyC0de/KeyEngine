#pragma once

#include <DirectXMath.h>
#include "constant_buffer.h"


class Camera;

class SkyboxVCB
	: public IBindable
{
	struct Transform
	{
		DirectX::XMMATRIX viewProjection;
	};
	std::unique_ptr<VertexConstantBuffer<Transform>> m_pVcb;
public:
	SkyboxVCB( Graphics& gph, unsigned slot = 0u );

	void bind( Graphics& gph ) cond_noex override;
	Transform getTransform( Graphics& gph ) cond_noex;
};