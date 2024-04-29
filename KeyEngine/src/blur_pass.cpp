#include "blur_pass.h"
#include "pixel_shader.h"
#include "texture_sampler_state.h"


namespace ren
{

BlurPass::BlurPass( Graphics &gph,
	const std::string &name )
	:
	IFullscreenPass{gph, name}
{
	addPassBindable( TextureSamplerState::fetch( gph, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

	addPassBindable( PixelShader::fetch( gph, "blur_ps.cso" ) );
}

void BlurPass::reset() cond_noex
{
	pass_;
}


}//namespace ren