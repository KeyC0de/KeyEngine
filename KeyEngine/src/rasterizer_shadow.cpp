#include "rasterizer_shadow.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


RasterizerShadow::RasterizerShadow( Graphics& gph,
	int depthBias,
	float slopeBias,
	float clamp )
{
	setParameters( gph,
		depthBias,
		slopeBias,
		clamp );
}

void RasterizerShadow::setParameters( Graphics& gph,
	int depthBias,
	float slopeBias,
	float clamp )
{
	this->m_depthBias = depthBias;
	this->m_slopeScaleDepthBias = slopeBias;
	this->m_clamp = clamp;

	D3D11_RASTERIZER_DESC rasterizerDesc{CD3D11_RASTERIZER_DESC{CD3D11_DEFAULT{}}};
	rasterizerDesc.DepthBias = depthBias;
	rasterizerDesc.SlopeScaledDepthBias = slopeBias;
	rasterizerDesc.DepthBiasClamp = clamp;

	HRESULT hres = getDevice( gph )->CreateRasterizerState( &rasterizerDesc,
		&m_pRasterizerState );
	ASSERT_HRES_IF_FAILED;
}

int RasterizerShadow::getDepthBias() const
{
	return m_depthBias;
}

float RasterizerShadow::getSlopeBias() const
{
	return m_slopeScaleDepthBias;
}

float RasterizerShadow::getClamp() const
{
	return m_clamp;
}

void RasterizerShadow::bind( Graphics& gph ) cond_noex
{
	getContext( gph )->RSSetState( m_pRasterizerState.Get() );
	DXGI_GET_QUEUE_INFO( gph );
}