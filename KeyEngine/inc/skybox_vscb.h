#pragma once

#include <DirectXMath.h>
#include "constant_buffer.h"


class Camera;

class SkyboxVSCB
	: public IBindable
{
	struct Transform
	{
		DirectX::XMMATRIX viewProjection;
	};
	std::unique_ptr<VertexShaderConstantBuffer<Transform>> m_pVscb;
public:
	SkyboxVSCB( Graphics& gph, unsigned slot = 0u );

	void bind( Graphics& gph ) cond_noex override;
	Transform getTransform( Graphics& gph ) cond_noex;
};