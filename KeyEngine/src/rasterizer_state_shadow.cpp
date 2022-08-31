#include "rasterizer_state_shadow.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"


RasterizerStateShadow::RasterizerStateShadow( Graphics &gph,
	const int depthBias,
	const float slopeBias,
	const float clamp )
	:
	m_depthBias{depthBias},
	m_slopeScaleDepthBias{slopeBias},
	m_clamp{clamp}
{
	D3D11_RASTERIZER_DESC rasterizerDesc{CD3D11_RASTERIZER_DESC{CD3D11_DEFAULT{}}};
	rasterizerDesc.DepthBias = depthBias;
	rasterizerDesc.SlopeScaledDepthBias = slopeBias;
	rasterizerDesc.DepthBiasClamp = clamp;

	HRESULT hres = getDevice( gph )->CreateRasterizerState( &rasterizerDesc,
		&m_pRasterizerState );
	ASSERT_HRES_IF_FAILED;
}

void RasterizerStateShadow::bind( Graphics &gph ) cond_noex
{
	getDeviceContext( gph )->RSSetState( m_pRasterizerState.Get() );
	DXGI_GET_QUEUE_INFO( gph );
}

void RasterizerStateShadow::setParameters( const int depthBias,
	const float slopeBias,
	const float clamp ) noexcept
{
	m_depthBias = depthBias;
	m_slopeScaleDepthBias = slopeBias;
	m_clamp = clamp;
}

const int RasterizerStateShadow::getDepthBias() const noexcept
{
	return m_depthBias;
}

const float RasterizerStateShadow::getSlopeScaleDepthBias() const noexcept
{
	return m_slopeScaleDepthBias;
}

const float RasterizerStateShadow::getClamp() const noexcept
{
	return m_clamp;
}