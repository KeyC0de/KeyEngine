#pragma once

#include "bindable.h"
#include "constant_buffer.h"


class Camera;

//=============================================================
//	\class	LightVSCB
//
//	\author	KeyC0de
//	\date	2022/02/19 22:35
//
//	\brief	VSCB for shadow casting light source usage (shadowing_vs.hlsli)
//=============================================================
class LightVSCB
	: public IBindable
{
	struct LightTransformVSCB
	{
		DirectX::XMMATRIX lightMatrix;
	};
	std::unique_ptr<VertexShaderConstantBuffer<LightTransformVSCB>> m_pVscb;
	const Camera *m_pPointLightShadowCamera = nullptr;
public:
	LightVSCB( Graphics &gph, unsigned slot );

	void update( Graphics &gph );
	void bind( Graphics &gph ) cond_noex override;
	void setCamera( const Camera *pCam ) noexcept;
};