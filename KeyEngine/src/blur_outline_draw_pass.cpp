#include "blur_outline_draw_pass.h"
#include "pixel_shader.h"
#include "vertex_shader.h"
#include "depth_stencil_state.h"
#include "linker.h"
#include "render_target.h"
#include "texture_sampler_state.h"
#include "rasterizer_state.h"


namespace ren
{

BlurOutlineDrawPass::BlurOutlineDrawPass( Graphics &gph,
	const std::string &name,
	const unsigned rezReductFactor )
	:
	RenderQueuePass{name}
{
	addPassBindable( VertexShader::fetch( gph, "flat_vs.cso" ) );
	addPassBindable( PixelShader::fetch( gph, "flat_ps.cso" ) );
	addPassBindable( DepthStencilState::fetch( gph, DepthStencilState::Mode::DepthOffStencilReadFF ) );

	const unsigned width = gph.getClientWidth() / rezReductFactor;
	const unsigned height = gph.getClientHeight() / rezReductFactor;
	// create a RTV to write (the PS operation - which performs a flat color shading) to an offscreen texture
	// next Pass we'll read from this RTV in the shader like a texture
	m_pRtv = std::make_shared<RenderTargetShaderInput>( gph, width, height, 0u );

#if defined _DEBUG && !defined NDEBUG
	const char *offscreenRtvBlurOutlineName = "OffscreenRenderTargetViewOutlineDraw";
	m_pRtv->d3dResourceCom()->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( offscreenRtvBlurOutlineName ), offscreenRtvBlurOutlineName );
#endif

	addLinker( BindableLinker<IRenderTargetView>::make( "offscreenBlurOutlineOut", m_pRtv ) );
}

void BlurOutlineDrawPass::run( Graphics &gph ) const cond_noex
{
	m_pRtv->clear( gph );
	RenderQueuePass::run( gph );
}


}