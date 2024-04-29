#include "pass_through.h"
#include "pixel_shader.h"
#include "texture_sampler_state.h"


namespace ren
{

PassThrough::PassThrough( Graphics &gph,
	const std::string &name )
	:
	IFullscreenPass{gph, name}
{
	addPassBindable( TextureSamplerState::fetch( gph, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

	addPassBindable( PixelShader::fetch( gph, "passthrough_ps.cso" ) );
}

void PassThrough::reset() cond_noex
{
	pass_;
}


}//namespace ren