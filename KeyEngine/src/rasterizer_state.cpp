#include "rasterizer_state.h"
#include "bindable_map.h"
#include "os_utils.h"
#include "dxgi_info_queue.h"
#include "bindable_exception.h"


RasterizerState::RasterizerState( Graphics &gfx,
	const RasterizerMode mode,
	const FillMode fillMode,
	const FaceMode faceMode,
	const int depthBias /*= D3D11_DEFAULT_DEPTH_BIAS*/,
	const float slopeBias /*= D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS*/,
	const float biasClamp /*= D3D11_DEFAULT_DEPTH_BIAS_CLAMP*/ )
	:
	m_mode{mode},
	m_fillMode{fillMode},
	m_faceMode{faceMode},
	m_depthBias{depthBias},
	m_slopeBias{slopeBias},
	m_biasClamp{biasClamp}
{
	D3D11_RASTERIZER_DESC rasterDesc{CD3D11_RASTERIZER_DESC{CD3D11_DEFAULT{}}};
	// Defaults:
	//rasterDesc.FillMode = D3D11_FILL_SOLID;	// determines fill mode to use when rasterizing triangles. In wireframe mode it draws lines connecting the vertices. In solid mode it fills the triangles formed by the vertices. In Both cases adjacent vertices are not drawn.
	//rasterDesc.CullMode = D3D11_CULL_BACK;	// specifies that triangles facing a particular direction are not drawn - they are being culled
	//rasterDesc.FrontCounterClockwise = FALSE;	// FrontCounterClockwise = FALSE means that D3D considers a face to be front face if its vertex winding order is specified in a clockwise fashion
	//rasterDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;							// depth value added to a given pixel (used for coplanar polygons in 3d space particularly useful for shadows)
	//rasterDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;	// scalar bias value on a given pixel's slope
	//rasterDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;				// maximum depth bias of a pixel
	//rasterDesc.DepthClipEnable = TRUE;		// enables clipping based on distance
	//rasterDesc.ScissorEnable = FALSE;			// enables scissor-rectangle culling; all pixels outside an active scissor rectangle are culled
	//rasterDesc.MultisampleEnable = FALSE;		// set to TRUE to use the quadrilateral line anti-aliasing algorithm, or set to FALSE to use the alpha line anti-aliasing algorithm. Both have an effect only on multisample antialiasing (MSAA) render targets
	//rasterDesc.AntialiasedLineEnable = FALSE;	// enables line antialiasing; only applies if doing line drawing and MultisampleEnable is FALSE

	if ( mode == DefaultRS )
	{
		rasterDesc.FillMode = fillMode == FillMode::Solid ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME;
		rasterDesc.CullMode = faceMode == FaceMode::Both ? D3D11_CULL_NONE :
			( faceMode == FaceMode::Front ? D3D11_CULL_BACK : D3D11_CULL_FRONT );
	}
	else if ( mode == ShadowRS )
	{
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.DepthBias = depthBias;
		rasterDesc.SlopeScaledDepthBias = slopeBias;
		rasterDesc.DepthBiasClamp = biasClamp;
	}
	else
	{
		THROW_BINDABLE_EXCEPTION( "Invalid Rasterizer mode." );
	}

	HRESULT hres = getDevice( gfx )->CreateRasterizerState( &rasterDesc, &m_pRasterizerState );
	ASSERT_HRES_IF_FAILED;
}

void RasterizerState::bind( Graphics &gfx ) cond_noex
{
	getDeviceContext( gfx )->RSSetState( m_pRasterizerState.Get() );
	DXGI_GET_QUEUE_INFO( gfx );
}

std::shared_ptr<RasterizerState> RasterizerState::fetch( Graphics &gfx,
	const RasterizerMode mode,
	const FillMode fillMode,
	const FaceMode faceMode,
	const int depthBias /*= D3D11_DEFAULT_DEPTH_BIAS*/,
	const float slopeBias /*= D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS*/,
	const float biasClamp /*= D3D11_DEFAULT_DEPTH_BIAS_CLAMP*/ )
{
	return BindableMap::fetch<RasterizerState>( gfx, mode, fillMode, faceMode, depthBias, slopeBias, biasClamp );
}

std::string RasterizerState::calcUid( const RasterizerMode mode,
	const FillMode fillMode,
	const FaceMode faceMode,
	const int depthBias /*= D3D11_DEFAULT_DEPTH_BIAS*/,
	const float slopeBias /*= D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS*/,
	const float biasClamp /*= D3D11_DEFAULT_DEPTH_BIAS_CLAMP*/ )
{
	using namespace std::string_literals;
	std::string modeId;
	std::string fillModeId;
	std::string faceModeId;
	std::string depthBiasId;
	std::string slopeBiasId;
	std::string biasClampId;
	switch ( mode )
	{
	case DefaultRS:
		modeId = "Def"s;
		depthBiasId = "0"s;
		slopeBiasId = "0"s;
		biasClampId = "0"s;
		break;
	case ShadowRS:
		modeId = "Sha"s;
		depthBiasId = std::to_string( depthBias );
		slopeBiasId = std::to_string( slopeBias );
		biasClampId = std::to_string( biasClamp );
		break;
	}
	switch ( fillMode )
	{
	case Front:
		fillModeId = "FS"s;
		break;
	case Back:
		fillModeId = "BS"s;
		break;
	case Both:
		fillModeId = "2S"s;
		break;
	}
	switch ( faceMode )
	{
	case Solid:
		faceModeId = "S"s;
		break;
	case Wireframe:
		faceModeId = "W"s;
		break;
	}
	return typeid( RasterizerState ).name() + "#"s + modeId + ":"s + fillModeId + ":"s + faceModeId + "|" + depthBiasId + ":" + slopeBiasId + ":" + biasClampId;
}

std::string RasterizerState::getUid() const noexcept
{
	return calcUid( m_mode, m_fillMode, m_faceMode, m_depthBias, m_slopeBias, m_biasClamp );
}

RasterizerState::RasterizerMode RasterizerState::getMode() const noexcept
{
	return m_mode;
}

Microsoft::WRL::ComPtr<ID3D11RasterizerState>& RasterizerState::getD3dRasterizerState()
{
	return m_pRasterizerState;
}
