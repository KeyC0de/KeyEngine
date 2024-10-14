#include "horizontal_blur_pass.h"
#include "graphics.h"
#include "pixel_shader.h"
#include "texture_sampler_state.h"


namespace ren
{

HorizontalBlurPass::HorizontalBlurPass( Graphics &gfx,
	const std::string &name,
	const unsigned rezReductFactor )
	:
	IFullscreenPass{gfx, name}
{
	addBindable( PixelShader::fetch( gfx, "blur_separ_ps.cso" ) );
	addBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Trilinear, TextureSamplerState::AddressMode::Clamp ) );

	addBindableBinderUsingContainerIndex<PixelShaderConstantBufferEx>( "blurKernel" );
	addBinder( BindableBinder<PixelShaderConstantBufferEx>::make( "blurDirection", m_pPscbBlurDirection ) );
	// we will use the offscreen texture from the previous pass and bind it as input in this pass so we can blur it
	addBindableBinderUsingContainerIndex<IRenderTargetView>( "offscreenBlurOutlineIn" );

	const unsigned width = gfx.getClientWidth() / rezReductFactor;
	const unsigned height = gfx.getClientHeight() / rezReductFactor;
	// create a RTV to write (the PS operation - which performs the Horizontal Blur) to an offscreen texture; next Pass we'll read from it
	m_pRtv = std::make_shared<RenderTargetShaderInput>( gfx, width, height, 0u );

#if defined _DEBUG && !defined NDEBUG
	const char *offscreenRtvName = "OffscreenRenderTargetViewSeparatedBlurFilter";
	m_pRtv->d3dResourceCom()->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( offscreenRtvName ), offscreenRtvName );
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