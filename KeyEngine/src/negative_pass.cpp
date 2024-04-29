#include "negative_pass.h"
#include "pixel_shader.h"
#include "texture_sampler_state.h"


namespace ren
{

NegativePass::NegativePass( Graphics &gph,
	const std::string &name )
	:
	IFullscreenPass{gph, name}
{
	addPassBindable( TextureSamplerState::fetch( gph, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

	addPassBindable( PixelShader::fetch( gph, "negative_ps.cso" ) );
}

void NegativePass::reset() cond_noex
{
	pass_;
}


}//namespace ren