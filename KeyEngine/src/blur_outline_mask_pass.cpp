#include "blur_outline_mask_pass.h"
#include "vertex_shader.h"
#include "pixel_shader.h"
#include "depth_stencil_state.h"
#include "rasterizer_state.h"


namespace ren
{

BlurOutlineMaskPass::BlurOutlineMaskPass( Graphics &gfx,
	const std::string &name )
	:
	RenderQueuePass{name}
{
	addPassBindable( VertexShader::fetch( gfx, "flat_vs.cso" ) );
	addPassBindable( PixelShaderNull::fetch( gfx ) );
	addPassBindable( DepthStencilState::fetch( gfx, DepthStencilState::Mode::DepthOffStencilWriteFF ) );
	addPassBindable( RasterizerState::fetch( gfx, RasterizerState::RasterizerMode::DefaultRS, RasterizerState::FillMode::Solid, RasterizerState::FaceMode::Front ) );

	addBinder( RenderSurfaceBinder<IDepthStencilView>::make( "depthStencil", m_pDsv ) );

	addLinker( RenderSurfaceLinker<IDepthStencilView>::make( "depthStencil", m_pDsv ) );
}


}// ren