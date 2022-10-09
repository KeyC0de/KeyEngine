#include "blur_pass.h"
#include "pixel_shader.h"
#include "blend_state.h"
#include "texture_sampler_state.h"
#include "depth_stencil_state.h"
#include "rasterizer_state.h"


namespace ren
{

BlurPass::BlurPass( Graphics &gph,
	const std::string &name,
	const unsigned rezReductFactor )
	:
	IFullscreenPass{gph, name}
{
	addPassBindable( PixelShader::fetch( gph,
		"blur_ps.cso" ) );
	addPassBindable( TextureSamplerState::fetch( gph,
		0u,
		TextureSamplerState::FilterMode::Trilinear,
		TextureSamplerState::AddressMode::Clamp ) );
	addPassBindable( BlendState::fetch( gph,
		BlendState::Mode::NoBlend,
		0u ) );

	const unsigned width = gph.getClientWidth();
	const unsigned height = gph.getClientHeight();
	// create a SRV to read the main texture
	m_pRtv = std::make_unique<RenderTargetShaderInput>( gph,
		width,
		height,
		0u );
	addProducer( BindableProducer<IRenderTargetView>::make( "offscreenFullscreenBlurOut",
		m_pRtv ) );
}

//void BlurPass::run( Graphics &gph ) const cond_noex
//{
	//m_pRtv->clear( gph );
	//IFullscreenPass::run( gph );
//}

void BlurPass::reset() cond_noex
{
	pass_;
}


}// namespace ren