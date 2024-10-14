#include "solid_outline_mask_pass.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "depth_stencil_state.h"
#include "rasterizer_state.h"


namespace ren
{

SolidOutlineMaskPass::SolidOutlineMaskPass( Graphics &gfx,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addBindable( VertexShader::fetch( gfx, "flat_vs.cso" ) );
	addBindable( PixelShaderNull::fetch( gfx ) );
	addBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::DepthOffStencilWriteFF ) );
	addBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

	addBinder( RenderSurfaceBinder<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IRenderTargetView>::make( "renderTarget", m_pRtv ) );
	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}


}