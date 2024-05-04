#include "horizontal_blur_pass.h"
#include "pixel_shader.h"
#include "blend_state.h"
#include "texture_sampler_state.h"


namespace ren
{

HorizontalBlurPass::HorizontalBlurPass( Graphics &gfx,
	const std::string &name,
	const unsigned rezReductFactor )
	:
	IFullscreenPass{gfx, name}
{
	addPassBindable( PixelShader::fetch( gfx, "blur_separ_ps.cso" ) );
	addPassBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Trilinear, TextureSamplerState::AddressMode::Clamp ) );

	addBinder( Binder<PixelShaderConstantBufferEx>::make( "blurDirection", m_pPscbBlurDirection ) );
	// read from offscreen texture and Horizontally Blur it (separated filter)
	addContainerBindableBinder<IRenderTargetView>( "offscreenBlurOutlineIn" );
	addContainerBindableBinder<PixelShaderConstantBufferEx>( "blurKernel" );

	const unsigned width = gfx.getClientWidth() / rezReductFactor;
	const unsigned height = gfx.getClientHeight() / rezReductFactor;
	// create a RTV to write (the PS operation - which performs the Horizontal Blur) to an offscreen texture
	// next Pass we'll read from this RTV in the shader like a texture
	m_pRtv = std::make_shared<RenderTargetShaderInput>( gfx, width, height, 0u );

#if defined _DEBUG && !defined NDEBUG
	const char *offscreenRtvBlurOutlineName = "OffscreenRenderTargetViewSeparatedBlurFilter";
	m_pRtv->d3dResourceCom()->SetPrivateData( WKPDID_D3DDebugObjectName, strlen( offscreenRtvBlurOutlineName ), offscreenRtvBlurOutlineName );
#endif

	addLinker( BindableLinker<IRenderTargetView>::make( "offscreenBlurOutlineOut", m_pRtv ) );
}

void HorizontalBlurPass::run( Graphics &gfx ) const cond_noex
{
	m_pRtv->clear( gfx );
	auto pscb = m_pPscbBlurDirection->getBufferCopy();
	pscb["bHorizontal"] = true;
	m_pPscbBlurDirection->setBuffer( pscb );
	m_pPscbBlurDirection->bind( gfx );
	IFullscreenPass::run( gfx );
}

void HorizontalBlurPass::reset() cond_noex
{
	pass_;
}


}// namespace ren