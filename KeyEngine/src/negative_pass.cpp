#include "negative_pass.h"
#include "pixel_shader.h"
#include "texture_sampler_state.h"
#include "blend_state.h"


namespace ren
{

NegativePass::NegativePass( Graphics &gfx,
	const std::string &name )
	:
	IFullscreenPass{gfx, name}
{
	// disable Blend State here for any later UI Passes
	addPassBindable( BlendState::fetch( gfx, BlendState::Mode::NoBlend, 0u ) );

	addPassBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

	addPassBindable( PixelShader::fetch( gfx, "negative_ps.cso" ) );
}

void NegativePass::reset() cond_noex
{
	pass_;
}


}//namespace ren