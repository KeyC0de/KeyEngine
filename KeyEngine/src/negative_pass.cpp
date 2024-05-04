#include "negative_pass.h"
#include "pixel_shader.h"
#include "texture_sampler_state.h"


namespace ren
{

NegativePass::NegativePass( Graphics &gfx,
	const std::string &name )
	:
	IFullscreenPass{gfx, name}
{
	addPassBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

	addPassBindable( PixelShader::fetch( gfx, "negative_ps.cso" ) );
}

void NegativePass::reset() cond_noex
{
	pass_;
}


}//namespace ren