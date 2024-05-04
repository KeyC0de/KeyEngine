#include "pass_through.h"
#include "pixel_shader.h"
#include "texture_sampler_state.h"


namespace ren
{

PassThrough::PassThrough( Graphics &gfx,
	const std::string &name )
	:
	IFullscreenPass{gfx, name}
{
	addPassBindable( TextureSamplerState::fetch( gfx, TextureSamplerState::TextureSamplerMode::DefaultTS, TextureSamplerState::FilterMode::Anisotropic, TextureSamplerState::AddressMode::Wrap ) );

	addPassBindable( PixelShader::fetch( gfx, "passthrough_ps.cso" ) );
}

void PassThrough::reset() cond_noex
{
	pass_;
}


}//namespace ren