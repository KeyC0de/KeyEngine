#pragma once

#include "bindable.h"
#include "constant_buffer.h"


class Camera;

class LightVCB
	: public IBindable
{
	struct LightTransformVCB
	{
		DirectX::XMMATRIX lightMatrix;
	};
	std::unique_ptr<VertexConstantBuffer<LightTransformVCB>> m_pVcb;
	const Camera* m_pPointLightShadowCamera = nullptr;
public:
	LightVCB( Graphics& gph, unsigned slot );

	void update( Graphics& gph );
	void bind( Graphics& gph ) cond_noex override;
	void setCamera( const Camera* pCam ) noexcept;
};