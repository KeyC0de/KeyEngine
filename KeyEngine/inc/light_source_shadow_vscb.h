#pragma once

#include "bindable.h"
#include "constant_buffer.h"


class Camera;

//=============================================================
//	\class	PointLightSourceShadowVSCB
//	\author	KeyC0de
//	\date	2022/02/19 22:35
//	\brief	VSCB for a Point light source for the purposes of shadowing, used to pass its location to GPU/HLSL(shadowing_vs.hlsli)
//=============================================================
// #TODO: make this abstract class and inherit from it for different light source types and allow the light to turn shadows on/off by calling PointLightSourceShadowVSCB::setCamera
class PointLightSourceShadowVSCB
	: public IBindable
{
	struct PointLightSourceShadowTransformVSCB
	{
		DirectX::XMMATRIX lightMatrix;
	};
	std::unique_ptr<VertexShaderConstantBuffer<PointLightSourceShadowTransformVSCB>> m_pVscb;
	const Camera *m_pPointLightShadowCamera = nullptr;
public:
	PointLightSourceShadowVSCB( Graphics &gph, const unsigned slot );

	void update( Graphics &gph );
	void bind( Graphics &gph ) cond_noex override;
	void setCamera( const Camera *pCam, const bool bEnable ) noexcept;
	bool isCastingShadows() const noexcept;
};