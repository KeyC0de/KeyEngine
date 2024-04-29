#include "solid_outline_mask_pass.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "depth_stencil_state.h"
#include "rasterizer_state.h"


namespace ren
{

SolidOutlineMaskPass::SolidOutlineMaskPass( Graphics &gph,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addPassBindable( VertexShader::fetch( gph, "flat_vs.cso" ) );
	addPassBindable( PixelShaderNull::fetch( gph ) );
	addPassBindable( DepthStencilState::fetch( gph, DepthStencilState::Mode::DepthOffStencilWriteFF ) );
	addPassBindable( RasterizerState::fetch( gph, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}


}