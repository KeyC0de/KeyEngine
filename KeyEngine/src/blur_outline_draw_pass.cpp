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

BlurOutlineDrawPass::BlurOutlineDrawPass( Graphics &gfx,
	const std::string &name,
	const unsigned rezReductFactor )
	:
	RenderQueuePass{name, {}, false}
{
	addPassBindable( VertexShader::fetch( gfx, "flat_vs.cso" ) );
	addPassBindable( PixelShader::fetch( gfx, "flat_ps.cso" ) );
	addPassBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::DepthOffStencilReadFF ) );

	const unsigned width = gfx.getClientWidth() / rezReductFactor;
	const unsigned height = gfx.getClientHeight() / rezReductFactor;
	// create a RTV to write (the PS operation - which performs a flat color shading) to an offscreen texture; next Pass we'll read from it
	m_pRtv = std::make_shared<RenderTargetShaderInput>( gfx, width, height, 0u );

#if defined _DEBUG && !defined NDEBUG
	const char *offscreenRtvName = "OffscreenRenderTargetViewBlurOutlineDraw";
	m_pRtv->d3dResourceCom()->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT) strlen( offscreenRtvName ), offscreenRtvName );
#endif

	addLinker( BindableLinker<IRenderTargetView>::make( "offscreenBlurOutlineOut", m_pRtv ) );
}

void BlurOutlineDrawPass::run( Graphics &gfx ) const cond_noex
{
	m_pRtv->clear( gfx );
	RenderQueuePass::run( gfx );
}


}